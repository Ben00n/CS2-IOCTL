#include "memory.h"
#include "helpers.h"
#include "render.h"
#include "offsets.h"
#include "vector.h"
#include "bone.h"
#include <dwmapi.h>
#include <d3d11.h>
#include <iostream>
#include <windowsx.h>


#include "../external/ImGui/imgui.h"
#include "../external/ImGui/imgui_impl_dx11.h"
#include "../external/ImGui/imgui_impl_win32.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK window_procedure(HWND window, UINT message, WPARAM w_param, LPARAM l_param)
{
    switch (message)
    {
    case WM_NCHITTEST:
    {
        const LONG borderWidth = GetSystemMetrics(SM_CXSIZEFRAME);
        const LONG titleBarHeight = GetSystemMetrics(SM_CYCAPTION);
        POINT cursorPos = { GET_X_LPARAM(l_param), GET_Y_LPARAM(l_param) };

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

    if (ImGui_ImplWin32_WndProcHandler(window, message, w_param, l_param))
    {
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

            uintptr_t gameScene = driver.read_memory<uintptr_t>(pCSPlayerPawn + 0x318);
            uintptr_t boneArray = driver.read_memory<uintptr_t>(gameScene + 0x160 + 0x80);

            int health = driver.read_memory<int>(pCSPlayerPawn + offsets::m_iHealth);

            if (health <= 0 || health > 100)
                continue;

            if (pCSPlayerPawn == localPlayer)
                continue;

            Vector3 origin = driver.read_memory<Vector3>(pCSPlayerPawn + offsets::m_vec_Origin);
            Vector3 head = driver.read_memory<Vector3>(boneArray + bones::head * 32);

            Vector3 screenPos = origin.World_To_Screen(view_matrix);
            Vector3 screenHead = head.World_To_Screen(view_matrix);

            float height = screenPos.y - screenHead.y;
            float width = height / 2.4f;

            RGB enemyColor = { 255, 0, 255 };

            for (int i = 0; i < sizeof(boneConnections) / sizeof(boneConnections[0]); i++)
            {
                int bone1 = boneConnections[i].bone1;
                int bone2 = boneConnections[i].bone2;

                Vector3 vectorBone1 = driver.read_memory<Vector3>(boneArray + bone1 * 32);
                Vector3 vectorBone2 = driver.read_memory<Vector3>(boneArray + bone2 * 32);

                Vector3 boneInWorld1 = vectorBone1.World_To_Screen(view_matrix);
                Vector3 boneInWorld2 = vectorBone2.World_To_Screen(view_matrix);


                Render::DrawLine(boneInWorld1.x, boneInWorld2.x, boneInWorld1.y, boneInWorld2.y, 1.4f, enemyColor);
            }
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