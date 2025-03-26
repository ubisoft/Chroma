#pragma once

#include "pch.h"

class SimpleCapture;

class App
{
public:

    void Initialize(winrt::Windows::UI::Composition::ContainerVisual const& root);

    void StartCapture(HWND const& hwnd, HWND const& clienthwnd, int const& filter);

    void WindowResized(HWND const& hwnd, HWND const& clienthwnd, int const& keyPress);

private:
    winrt::Windows::UI::Composition::Compositor m_compositor{ nullptr };
    winrt::Windows::UI::Composition::ContainerVisual m_root{ nullptr };
    winrt::Windows::UI::Composition::SpriteVisual m_content{ nullptr };
    winrt::Windows::UI::Composition::CompositionSurfaceBrush m_brush{ nullptr };
    winrt::Windows::UI::Composition::ICompositionSurface m_surface{ nullptr };

    winrt::Windows::Graphics::DirectX::Direct3D11::IDirect3DDevice m_device{ nullptr };
    std::unique_ptr<SimpleCapture> m_capture{ nullptr };
    
};