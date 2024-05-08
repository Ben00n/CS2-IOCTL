#include "renderer.h"
#include <stdexcept>

Renderer::Renderer(HWND hwnd) {
    DXGI_SWAP_CHAIN_DESC sd{};
    sd.BufferDesc.RefreshRate.Numerator = 60U;
    sd.BufferDesc.RefreshRate.Denominator = 1U;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1U;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.BufferCount = 2U;
    sd.OutputWindow = hwnd;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    constexpr D3D_FEATURE_LEVEL levels[2]{
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_0,
    };

    if (FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0U, levels, 2U, D3D11_SDK_VERSION, &sd, &m_swapChain, &m_device, nullptr, &m_deviceContext))) {
        throw std::runtime_error("Failed to create D3D11 device and swap chain");
    }

    ID3D11Texture2D* backBuffer{ nullptr };
    if (FAILED(m_swapChain->GetBuffer(0U, IID_PPV_ARGS(&backBuffer)))) {
        throw std::runtime_error("Failed to get back buffer");
    }

    if (FAILED(m_device->CreateRenderTargetView(backBuffer, nullptr, &m_renderTargetView))) {
        throw std::runtime_error("Failed to create render target view");
    }
    backBuffer->Release();
}

Renderer::~Renderer() {
    if (m_renderTargetView) m_renderTargetView->Release();
    if (m_swapChain) m_swapChain->Release();
    if (m_deviceContext) m_deviceContext->Release();
    if (m_device) m_device->Release();
}

void Renderer::BeginFrame() {
    m_deviceContext->OMSetRenderTargets(1U, &m_renderTargetView, nullptr);

    constexpr float color[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
    m_deviceContext->ClearRenderTargetView(m_renderTargetView, color);
}

void Renderer::EndFrame() {
    m_swapChain->Present(0U, 0U);
}