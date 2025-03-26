#pragma once
#define NUMVERTICES 6

#include "../shaders/Vertex.h"
#include <iostream>
#include <ctime>
#include <ratio>
#include <chrono>
#include <mutex>

class SimpleCapture
{
public:
    SimpleCapture(
        winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice const& device,
        winrt::Windows::Graphics::Capture::GraphicsCaptureItem const& item,
        HWND const& clientHwnd,
        int const& filter);
    ~SimpleCapture() { Close(); }

    void StartCapture();
    winrt::Windows::UI::Composition::ICompositionSurface CreateSurface(
        winrt::Windows::UI::Composition::Compositor const& compositor);
   
    void Close();
    void CheckClosed();
    std::atomic<bool> m_closed = false;

private:
    void OnFrameArrived(
        winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool const& sender,
        winrt::Windows::Foundation::IInspectable const& args);

private:
    winrt::Windows::Graphics::Capture::GraphicsCaptureItem m_item{ nullptr };
    winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool m_framePool{ nullptr };
    winrt::Windows::Graphics::Capture::GraphicsCaptureSession m_session{ nullptr };
    winrt::Windows::Graphics::SizeInt32 m_lastSize;

    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device{ nullptr };
    IDXGIFactory2* m_Factory;
    ID3D11VertexShader* m_VertexShader;
    ID3D11PixelShader* m_PixelShader{ nullptr };
    ID3D11InputLayout* m_InputLayout;
    ID3D11BlendState* m_BlendState;
    ID3D11RenderTargetView* m_RTV;
    ID3D11SamplerState* m_SamplerLinear;
    winrt::com_ptr<ID3D11DeviceContext> m_d3dContext{ nullptr };
    winrt::com_ptr<ID3D11Device> m_3dDevice{ nullptr };
    IDXGISwapChain1* m_SwapChain;
    ID3D11Texture2D* m_SharedSurf;
    HRESULT CreateSharedSurf();
    HRESULT SetViewPort(UINT Width, UINT Height);
    HRESULT InitShaders();
    HRESULT MakeRTV();
    
	winrt::Windows::Graphics::Capture::Direct3D11CaptureFramePool::FrameArrived_revoker m_frameArrived;
    HWND m_clientHwnd;
    int m_filter;

    std::chrono::high_resolution_clock::time_point current_ticks, last_ticks;
    
    std::string m_windowText;
    int counter = 0;
    long FPSTotal = 0;

    std::mutex m; 
};