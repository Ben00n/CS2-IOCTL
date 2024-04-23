#include "memory.h"
#include "helpers.h"
#include <iostream>

#include "vector.h"
#include <dwmapi.h>
#include <d3d11.h>
#include <windowsx.h>
#include "../external/ImGui/imgui.h"
#include "../external/ImGui/imgui_impl_dx11.h"
#include "../external/ImGui/imgui_impl_win32.h"
#include "render.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace offsets
{
    constexpr std::ptrdiff_t dwLocalPlayer = 0x17361C8;
    constexpr std::ptrdiff_t dwEntityList = 0x18C1EA8;
    constexpr std::ptrdiff_t dwViewMatrix = 0x1923180;

    constexpr std::ptrdiff_t m_iHealth = 0x334;
    constexpr std::ptrdiff_t dwPlayerPawn = 0x7E4;
    constexpr std::ptrdiff_t m_iTeamNum = 0x3CB;
    constexpr std::ptrdiff_t m_vec_Origin = 0x127C;
}


LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
    {
        return 0L;
    }

    if (message == WM_DESTROY)
    {
        PostQuitMessage(0);
        return 0L;
    }

    switch (message)
    {
    case WM_NCHITTEST:
    {
        const LONG borderWidth = GetSystemMetrics(SM_CXSIZEFRAME);
        const LONG titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
        POINT cursorPos = { GET_X_LPARAM(w_param), GET_Y_LPARAM(l_param) };
        RECT windowRect;
        GetWindowRect(window, &windowRect);

        if (cursorPos.y >= windowRect.top && cursorPos.y < windowRect.top + titleBarHeight)
            return HTCAPTION;

        break;
    }
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(window, message, w_param, l_param);
}


INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, int cmd_show)
{
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = window_procedure;
    wc.hInstance = instance;
    wc.lpszClassName = L"Benoon";

    RegisterClassExW(&wc);

    const HWND overlay = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED, wc.lpszClassName, L"Benoon", WS_POPUP, 0, 0, screenWidth, screenHeight, nullptr, nullptr, wc.hInstance, nullptr);

    SetLayeredWindowAttributes(overlay, RGB(0, 0, 0), BYTE(255), LWA_ALPHA);

    {
        RECT client_area{};
        GetClientRect(overlay, &client_area);

        RECT window_area{};
        GetWindowRect(overlay, &window_area);

        POINT diff{};
        ClientToScreen(overlay, &diff);

        const MARGINS margins{
            window_area.left + (diff.x - window_area.left),
            window_area.top + (diff.y - window_area.top),
            client_area.right,
            client_area.bottom,
        };

        DwmExtendFrameIntoClientArea(overlay, &margins);
    }

    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.RefreshRate.Numerator = 60U; // fps
    sd.BufferDesc.RefreshRate.Denominator = 1U;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1U;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2U;
    sd.OutputWindow = overlay;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2]{
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    ID3D11Device* device{ nullptr };
    ID3D11DeviceContext* device_context{ nullptr };
    IDXGISwapChain* swap_chain{ nullptr };
    ID3D11RenderTargetView* render_target_view{ nullptr };
    D3D_FEATURE_LEVEL level{};

    D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0U, levels, 2U, D3D11_SDK_VERSION, &sd, &swap_chain, &device, &level, &device_context);

    ID3D11Texture2D* back_buffer{ nullptr };
    swap_chain->GetBuffer(0U, IID_PPV_ARGS(&back_buffer));

    if (back_buffer)
    {
        device->CreateRenderTargetView(back_buffer, nullptr, &render_target_view);
        back_buffer->Release();
    }
    else
        return 1;

    ShowWindow(overlay, cmd_show);
    UpdateWindow(overlay);

    ImGui::CreateContext();
    ImGui::StyleColorsClassic();

    ImGui_ImplWin32_Init(overlay);
    ImGui_ImplDX11_Init(device, device_context);

    bool on = true;

    std::cout << "Benoon Says Hello!\n";

    const DWORD pid = get_process_id(L"cs2.exe");
    const uintptr_t module = get_module_base(pid, L"client.dll");

    if (pid == 0) {
        std::cout << "Failed to find cs2\n";
        std::cin.get();
        return 1;
    }

    const HANDLE driverHandle = CreateFileW(L"\\\\.\\BenoonDriver", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (driverHandle == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to create driver handle.\n";
        std::cin.get();
        return 1;
    }

    Memory driver(driverHandle, pid);

    while (on)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                on = false;
            }
        }

        if (!on)
            break;

        uintptr_t localPlayer = driver.read_memory<uintptr_t>(module + offsets::dwLocalPlayer);
        Vector3 localOrigin = driver.read_memory<Vector3>(localPlayer + offsets::m_vec_Origin);
        view_matrix_t view_matrix = driver.read_memory<view_matrix_t>(module + offsets::dwViewMatrix);
        uintptr_t entity_list = driver.read_memory<uintptr_t>(module + offsets::dwEntityList);
        int localTeam = driver.read_memory<int>(localPlayer + offsets::m_iTeamNum);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        for (int playerIndex = 1; playerIndex <= 31; ++playerIndex)
        {
            uintptr_t listen = driver.read_memory<uintptr_t>(entity_list + (8 * (playerIndex & 0x7FFF) >> 9) + 16);
            if (!listen)
                continue;

            uintptr_t player = driver.read_memory<uintptr_t>(listen + 120 * (playerIndex & 0x1FF));

            if (!player)
                continue;

            int playerTeam = driver.read_memory<int>(player + offsets::m_iTeamNum);

            if (playerTeam == localTeam)
                continue;

            uint32_t playerPawn = driver.read_memory<uint32_t>(player + offsets::dwPlayerPawn);
            uintptr_t listen2 = driver.read_memory<uintptr_t>(entity_list + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);

            if (!listen2)
                continue;

            uintptr_t pCSPlayerPawn = driver.read_memory<uintptr_t>(listen2 + 120 * (playerPawn & 0x1FF));

            if (!pCSPlayerPawn)
                continue;

            int health = driver.read_memory<int>(pCSPlayerPawn + offsets::m_iHealth);

            if (health <= 0 || health > 100)
                continue;

            if (pCSPlayerPawn == localPlayer)
                continue;

            Vector3 origin = driver.read_memory<Vector3>(pCSPlayerPawn + offsets::m_vec_Origin);
            Vector3 head = { origin.x, origin.y, origin.z + 75.f };

            Vector3 screenPos = origin.World_To_Screen(view_matrix);
            Vector3 screenHead = head.World_To_Screen(view_matrix);

            float height = screenPos.y - screenHead.y;
            float width = height / 2.4f;

            RGB enemyColor = { 255, 0, 255 };

            Render::DrawRect(
                screenHead.x - width / 2,
                screenHead.y,
                width,
                height,
                enemyColor,
                1.5
            );
        }

        ImGui::Render();
        float color[4]{ 0,0,0,0 };
        device_context->OMSetRenderTargets(1U, &render_target_view, nullptr);
        device_context->ClearRenderTargetView(render_target_view, color);

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        swap_chain->Present(0U, 0U);
    }

    // exit
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();

    if (swap_chain) swap_chain->Release();
    if (device_context) device_context->Release();
    if (device) device->Release();
    if (render_target_view) render_target_view->Release();

    DestroyWindow(overlay);
    UnregisterClassW(wc.lpszClassName, wc.hInstance);
    CloseHandle(driverHandle);

    return 0;
}


//int main()
//{
//    std::cout << "Benoon!\n";
//
//    const DWORD pid = get_process_id(L"cs2.exe");
//    const uintptr_t module = get_module_base(pid, L"client.dll");
//
//    if (pid == 0) {
//        std::cout << "Failed to find cs2\n";
//        std::cin.get();
//        return 1;
//    }
//
//    const HANDLE driverHandle = CreateFileW(L"\\\\.\\BenoonDriver", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
//    if (driverHandle == INVALID_HANDLE_VALUE) {
//        std::cout << "Failed to create driver handle.\n";
//        std::cin.get();
//        return 1;
//    }
//
//    Memory driver(driverHandle, pid);
//
//    int value = driver.read_memory<int>(0x02F42380);
//    std::cout << value;
//    std::cin.get();
//
//    CloseHandle(driverHandle);
//
//    std::cin.get();
//
//    return 0;
//}