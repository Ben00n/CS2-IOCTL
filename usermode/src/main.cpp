#include "memory.h"
#include "helpers.h"
#include "render.h"
#include "offsets.h"
#include "vector.h"
#include "bone.h"
#include <iostream>
#include "../external/ImGui/imgui.h"
#include "../external/ImGui/imgui_impl_dx11.h"
#include "../external/ImGui/imgui_impl_win32.h"
#include "overlay.h"
#include "renderer.h"

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, int cmd_show) {
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    Overlay overlay(instance, L"Benoon", L"Benoon", screenWidth, screenHeight);
    Renderer renderer(overlay.GetWindowHandle());

    ImGui::CreateContext();
    ImGui::StyleColorsClassic();

    ImGui_ImplWin32_Init(overlay.GetWindowHandle());
    ImGui_ImplDX11_Init(renderer.GetDevice(), renderer.GetDeviceContext());

    bool on = true;

    const DWORD pid = get_process_id(L"cs2.exe");
    const uintptr_t module = get_module_base(pid, L"client.dll");

    if (pid == 0) {
        std::cout << "Failed to find cs2\n";
        std::cin.get();
        return 1;
    }

    const HANDLE driverHandle = CreateFileW(L"\\\\.\\BenoonDriver", GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (driverHandle == INVALID_HANDLE_VALUE) {
        std::cout << "Failed to create driver handle.\n";
        std::cin.get();
        return 1;
    }

    Memory driver(driverHandle, pid);

    const auto boneConnectionsSize = sizeof(boneConnections) / sizeof(boneConnections[0]);
    const RGB enemyColor = { 255, 255, 255 };

    while (on) {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT) {
                on = false;
            }
        }

        if (!on)
            break;

        uintptr_t localPlayer = driver.read_memory<uintptr_t>(module + offsets::dwLocalPlayer);
        view_matrix_t view_matrix = driver.read_memory<view_matrix_t>(module + offsets::dwViewMatrix);
        uintptr_t entity_list = driver.read_memory<uintptr_t>(module + offsets::dwEntityList);
        int localTeam = driver.read_memory<int>(localPlayer + offsets::m_iTeamNum);

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        renderer.BeginFrame();

        for (int playerIndex = 1; playerIndex <= 31; ++playerIndex) {
            uintptr_t listen = driver.read_memory<uintptr_t>(entity_list + (8 * (playerIndex & 0x7FFF) >> 9) + 16);
            if (!listen) continue;

            uintptr_t player = driver.read_memory<uintptr_t>(listen + 120 * (playerIndex & 0x1FF));
            if (!player) continue;

            int playerTeam = driver.read_memory<int>(player + offsets::m_iTeamNum);
            if (playerTeam == localTeam) continue;

            uint32_t playerPawn = driver.read_memory<uint32_t>(player + offsets::dwPlayerPawn);
            uintptr_t listen2 = driver.read_memory<uintptr_t>(entity_list + 0x8 * ((playerPawn & 0x7FFF) >> 9) + 16);
            if (!listen2) continue;

            uintptr_t pCSPlayerPawn = driver.read_memory<uintptr_t>(listen2 + 120 * (playerPawn & 0x1FF));
            if (!pCSPlayerPawn || pCSPlayerPawn == localPlayer) continue;

            uintptr_t gameScene = driver.read_memory<uintptr_t>(pCSPlayerPawn + 0x318);
            uintptr_t boneArray = driver.read_memory<uintptr_t>(gameScene + 0x160 + 0x80);

            int health = driver.read_memory<int>(pCSPlayerPawn + offsets::m_iHealth);
            if (health <= 0 || health > 100) continue;

            Vector3 head = driver.read_memory<Vector3>(boneArray + bones::head * 32);
            Vector3 screenHead = head.World_To_Screen(view_matrix);
            if (screenHead.z <= 0.01f) continue;

            char healthText[10];
            snprintf(healthText, sizeof(healthText), "%d", health);

            Vector3 labelPos = head;
            labelPos.z += 30.0f;
            Vector3 screenLabelPos = labelPos.World_To_Screen(view_matrix);
            if (screenLabelPos.z > 0.01f)
                Render::DrawLabel(healthText, screenLabelPos.x, screenLabelPos.y, RGB{ 255, 0, 0 }, true);

            char playerNameBuffer[128];
            memset(playerNameBuffer, 0, sizeof(playerNameBuffer));

            for (int i = 0; i < sizeof(playerNameBuffer) - 1; ++i) {
                char c = driver.read_memory<char>(player + offsets::m_iszPlayerName + i);
                if (c == '\0' || c == ' ') break;
                playerNameBuffer[i] = c;
            }

            if (playerNameBuffer[0] != '\0') {
                Vector3 nameLabelPos = head;
                nameLabelPos.z += 50.0f;
                Vector3 screenNameLabelPos = nameLabelPos.World_To_Screen(view_matrix);
                if (screenNameLabelPos.z > 0.01f)
                    Render::DrawLabel(playerNameBuffer, screenNameLabelPos.x, screenNameLabelPos.y, RGB{ 0, 255, 255 }, true);
            }

            for (int i = 0; i < boneConnectionsSize; i++) {
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
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        renderer.EndFrame();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CloseHandle(driverHandle);

    return 0;
}