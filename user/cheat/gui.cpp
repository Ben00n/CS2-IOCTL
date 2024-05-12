#include "gui.h"
#include "globals.h"

#include "../external/ImGui/imgui.h"
#include "../external/ImGui/imgui_impl_dx11.h"
#include "../external/ImGui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND window, UINT message, WPARAM wideParam, LPARAM longParam);

LRESULT CALLBACK WindowProcess(HWND window, UINT message, WPARAM wideParam, LPARAM longParam)
{
    if (ImGui_ImplWin32_WndProcHandler(window, message, wideParam, longParam))
        return true;

    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    case WM_SYSCOMMAND:
    {
        if ((wideParam & 0xfff0) == SC_KEYMENU) // disable ALT app menu
            return 0;
        break;
    }
    case WM_LBUTTONDOWN:
    {
        gui::position = MAKEPOINTS(longParam);
        return 0;
    }
    case WM_MOUSEMOVE:
    {
        if (wideParam == MK_LBUTTON)
        {
            const auto points = MAKEPOINTS(longParam);
            auto rect = ::RECT{ };

            GetWindowRect(gui::window, &rect);

            rect.left += points.x - gui::position.x;
            rect.top += points.y - gui::position.y;

            if (gui::position.x >= 0 && gui::position.x <= gui::WIDTH && gui::position.y >= 0 && gui::position.y <= 19)
                SetWindowPos(gui::window, HWND_TOPMOST, rect.left, rect.top, 0, 0, SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER);
        }
        return 0;
    }
    }

    return DefWindowProcW(window, message, wideParam, longParam);
}


void gui::CreateHWindow(const wchar_t* windowName, const wchar_t* className) noexcept
{
    windowClass.cbSize = sizeof(WNDCLASSEXA);
    windowClass.style = CS_CLASSDC;
    windowClass.lpfnWndProc = WindowProcess;
    windowClass.cbClsExtra = 0;
    windowClass.cbWndExtra = 0;
    windowClass.hInstance = GetModuleHandleW(0);
    windowClass.hIcon = 0;
    windowClass.hCursor = 0;
    windowClass.hbrBackground = 0;
    windowClass.lpszMenuName = 0;
    windowClass.lpszClassName = className;
    windowClass.hIconSm = 0;

    RegisterClassExW(&windowClass);

    window = CreateWindowW(className, windowName, WS_POPUP, 100, 100, WIDTH, HEIGHT, 0, 0, windowClass.hInstance, 0);

    ShowWindow(window, SW_SHOWDEFAULT);
    UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
    DestroyWindow(window);
    UnregisterClassW(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
    DXGI_SWAP_CHAIN_DESC swapChainDesc;
    ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = gui::WIDTH;
    swapChainDesc.BufferDesc.Height = gui::HEIGHT;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = gui::window;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;

    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };

    HRESULT result = D3D11CreateDeviceAndSwapChain(
        nullptr,
        D3D_DRIVER_TYPE_HARDWARE,
        nullptr,
        0,
        featureLevels,
        _countof(featureLevels),
        D3D11_SDK_VERSION,
        &swapChainDesc,
        &gui::swapChain,
        &gui::device,
        &featureLevel,
        &gui::deviceContext
    );

    if (FAILED(result))
        return false;

    ID3D11Texture2D* backBuffer;
    gui::swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));

    if (backBuffer)
    {
        gui::device->CreateRenderTargetView(backBuffer, nullptr, &gui::renderTargetView);
        backBuffer->Release();
    }
    else
    {
        return false;
    }

    return true;
}

void gui::ResetDevice() noexcept
{
    ImGui_ImplDX11_InvalidateDeviceObjects();

    HRESULT hr = swapChain->ResizeBuffers(0, 0, 0, DXGI_FORMAT_UNKNOWN, 0);
    IM_ASSERT(hr == S_OK);

    ID3D11Texture2D* backBuffer;
    hr = swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    IM_ASSERT(hr == S_OK);

    if (renderTargetView) { renderTargetView->Release(); renderTargetView = nullptr; }
    hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
    IM_ASSERT(hr == S_OK);
    backBuffer->Release();

    ImGui_ImplDX11_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
    if (renderTargetView) { renderTargetView->Release(); renderTargetView = nullptr; }
    if (swapChain) { swapChain->Release(); swapChain = nullptr; }
    if (deviceContext) { deviceContext->Release(); deviceContext = nullptr; }
    if (device) { device->Release(); device = nullptr; }
}

void gui::CreateImGui() noexcept
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ::ImGui::GetIO();

    io.IniFilename = nullptr;
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(window);
    ImGui_ImplDX11_Init(device, deviceContext);
}

void gui::DestroyImGui() noexcept
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
    MSG message;
    while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
    {
        TranslateMessage(&message);
        DispatchMessage(&message);
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
    ImGui::Render();

    deviceContext->OMSetRenderTargets(1, &renderTargetView, NULL);

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    deviceContext->ClearRenderTargetView(renderTargetView, clearColor);

    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    const auto result = swapChain->Present(1, 0);

    // Handle loss of D3D11 device
    if (result == DXGI_ERROR_DEVICE_REMOVED || result == DXGI_ERROR_DEVICE_RESET)
    {
        ResetDevice();
    }
}

void gui::Render() noexcept
{
    ImGui::SetNextWindowPos({ 0, 0 });
    ImGui::SetNextWindowSize({ WIDTH, HEIGHT });
    ImGui::Begin("Ben00n", &exit, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove);

    ImGui::Checkbox("Skeleton ESP", &globals::skeletonEsp);
    ImGui::ColorEdit4("Skeleton Color", globals::skeletonColor);

    ImGui::Checkbox("Health ESP", &globals::healthEsp);
    ImGui::ColorEdit4("Health Color", globals::healthColor);

    ImGui::End();
}