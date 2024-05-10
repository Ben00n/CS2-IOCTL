#include "gui.h"
#include <thread>

INT APIENTRY WinMain(HINSTANCE instance, HINSTANCE, PSTR, int cmd_show)
{
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