#include "overlay.h"
#include <dwmapi.h>
#include "../external/ImGui/imgui.h"
#include "../external/ImGui/imgui_impl_win32.h"

Overlay::Overlay(HINSTANCE hInstance, const wchar_t* className, const wchar_t* windowName, int width, int height)
{
    m_wc = { sizeof(WNDCLASSEXW), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hInstance, nullptr, nullptr, nullptr, nullptr, className, nullptr };
    RegisterClassExW(&m_wc);

    m_hwnd = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, className, windowName, WS_POPUP, 0, 0, width, height, nullptr, nullptr, hInstance, nullptr);

    SetLayeredWindowAttributes(m_hwnd, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    RECT clientArea{};
    GetClientRect(m_hwnd, &clientArea);

    RECT windowArea{};
    GetWindowRect(m_hwnd, &windowArea);

    POINT diff{};
    ClientToScreen(m_hwnd, &diff);

    const MARGINS margins 
    {
        windowArea.left + (diff.x - windowArea.left),
        windowArea.top + (diff.y - windowArea.top),
        clientArea.right,
        clientArea.bottom,
    };

    DwmExtendFrameIntoClientArea(m_hwnd, &margins);

    ShowWindow(m_hwnd, SW_SHOW);
    UpdateWindow(m_hwnd);
}

Overlay::~Overlay()
{
    DestroyWindow(m_hwnd);
    UnregisterClassW(m_wc.lpszClassName, m_wc.hInstance);
}

LRESULT CALLBACK Overlay::WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    if (ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam)) 
        return true;


    switch (message) {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProcW(hwnd, message, wParam, lParam);
    }
}