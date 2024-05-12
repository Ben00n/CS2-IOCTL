#include "gui.h"
#include <thread>
#include <memory.h>
#include <helpers.h>

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, int cmd_show)
{
	const DWORD pid = get_process_id(L"cs2.exe");
	const uintptr_t module = get_module_base(pid, L"client.dll");

	if (pid == 0) {
		return 1;
	}

	const HANDLE driverHandle = CreateFileW(L"\\\\.\\BenoonDriver", GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (driverHandle == INVALID_HANDLE_VALUE) {
		return 1;
	}

	Memory driver(driverHandle, pid);

	gui::CreateHWindow(L"Ben00n Menu", L"Ben00n Class");
	gui::CreateDevice();
	gui::CreateImGui();

	while (gui::exit)
	{
		gui::BeginRender();
		gui::Render();
		gui::EndRender();

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	gui::DestroyImGui();
	gui::DestroyDevice();
	gui::DestroyHWindow();

	return EXIT_SUCCESS;
}