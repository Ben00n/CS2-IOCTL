#pragma once
#include <d3d11.h>

namespace gui
{
	constexpr int WIDTH = 500;
	constexpr int HEIGHT = 300;

	// state of our window
	inline bool exit = true;

	// winapi window vars
	inline HWND window = nullptr;
	inline WNDCLASSEXW windowClass = { };

	// points for window movement
	inline POINTS position = { };

	// D3D11 vars
	inline ID3D11Device* device = nullptr;
	inline ID3D11DeviceContext* deviceContext = nullptr;
	inline IDXGISwapChain* swapChain = nullptr;
	inline ID3D11RenderTargetView* renderTargetView = nullptr;
	
	// handle window creation and destruction
	void CreateHWindow(const wchar_t* windowName, const wchar_t* className) noexcept;
	void DestroyHWindow() noexcept;

	bool CreateDevice() noexcept;
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;

	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;

	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;
}