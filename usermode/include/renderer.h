#pragma once
#include <d3d11.h>
#include <Windows.h>

class Renderer {
public:
    Renderer(HWND hwnd);
    ~Renderer();

    void BeginFrame();
    void EndFrame();

    ID3D11Device* GetDevice() const { return m_device; }
    ID3D11DeviceContext* GetDeviceContext() const { return m_deviceContext; }

private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
    IDXGISwapChain* m_swapChain;
    ID3D11RenderTargetView* m_renderTargetView;
};