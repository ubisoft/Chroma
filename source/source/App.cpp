#include "../headers/pch.h"
#include "../headers/App.h"
#include "../headers/SimpleCapture.h"

using namespace winrt;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Windows::UI;
using namespace Windows::UI::Composition;
using namespace Windows::Graphics::Capture;
using namespace winrt::Windows::Graphics::Capture;

auto g_widthOffset = 0;
auto g_heightOffset = 0;

void App::Initialize(ContainerVisual const& root)
{
    auto queue = DispatcherQueue::GetForCurrentThread();

    m_compositor = root.Compositor();
    m_root = m_compositor.CreateContainerVisual();
    m_content = m_compositor.CreateSpriteVisual();
    m_brush = m_compositor.CreateSurfaceBrush();

    root.Children().InsertAtTop(m_root);

    m_content.AnchorPoint({ 0.5f, 0.5f });
    m_content.RelativeOffsetAdjustment({ 0.5f, 0.5f, 0 });
    m_content.Brush(m_brush);
    m_brush.HorizontalAlignmentRatio(0.5f);
    m_brush.VerticalAlignmentRatio(0.5f);
    m_root.Children().InsertAtTop(m_content);

    auto d3dDevice = CreateD3DDevice();
    auto dxgiDevice = d3dDevice.as<IDXGIDevice>();
    m_device = CreateDirect3DDevice(dxgiDevice.get());
}

void App::StartCapture(HWND const& hwnd, HWND const& clienthwnd, int const& filter)
{
	if (m_capture)
	{
        m_surface = nullptr;
		m_capture->Close();
		m_capture = nullptr;
	}
   
    WindowResized(hwnd, clienthwnd, -1);
    auto item = CreateCaptureItemForWindow(hwnd);
    if (!item)
    {
        return;
    }
    try
    {
        m_capture = std::make_unique<SimpleCapture>(m_device, item, clienthwnd, filter);
        m_surface = m_capture->CreateSurface(m_compositor);

        m_brush.Surface(m_surface);
        m_capture->StartCapture();
    }
    catch (winrt::hresult_error const& ex)
    {
        std::wstringstream wss;
        wss << L"Failed to initialize screen capture.\r\n";
        wss << L"Error Code: 0x" << std::hex << ex.code().value << L"\r\n";
        wss << L"Message: " << ex.message().c_str();
        MessageBoxW(clienthwnd, wss.str().c_str(), L"Capture Error", MB_OK | MB_ICONERROR);
        
        if (m_capture)
        {
            m_capture->Close(); // Ensure any partial resources are released
            m_capture = nullptr;
        }
        m_surface = nullptr;
        // Ensure brush is not holding onto a stale surface
        if(m_brush) { m_brush.Surface(nullptr); }
    }
    return;
}

void App::WindowResized(HWND const& hwnd, HWND const& clienthwnd, int const& keyPress)
{
    // get offsets from user key press
    switch (keyPress)
    {
    case VK_LEFT:
        g_widthOffset--;
        break;
    case VK_UP:
        g_heightOffset--;
        break;
    case VK_RIGHT:
        g_widthOffset++;
        break;
    case VK_DOWN:
        g_heightOffset++;
        break;
    default:
        break;
    }

    // get client and source window monitors
    auto clientMonitor = MonitorFromWindow(clienthwnd, MONITOR_DEFAULTTOPRIMARY);
    auto srcMonitor = MonitorFromWindow(hwnd, MONITOR_DEFAULTTONULL);
    MONITORINFOEX clientMonitorinfo;
    MONITORINFOEX srcMonitorinfo;
    clientMonitorinfo.cbSize = sizeof(MONITORINFOEX);
    srcMonitorinfo.cbSize = sizeof(MONITORINFOEX);
    
    if (!GetMonitorInfoW(clientMonitor, &clientMonitorinfo) || !GetMonitorInfoW(srcMonitor, &srcMonitorinfo))
    {
        return;
    }

    WCHAR* srcDeviceName = srcMonitorinfo.szDevice;
    WCHAR* clientDeviceName = clientMonitorinfo.szDevice;

    if (wcscmp(srcDeviceName, clientDeviceName) == 0)
    {
        RECT srcRect;
        GetWindowRect(hwnd, &srcRect);

        m_root.Size({ (float)(clientMonitorinfo.rcWork.right - clientMonitorinfo.rcWork.left), (float)(clientMonitorinfo.rcWork.bottom - clientMonitorinfo.rcWork.top) });
        m_content.Size({ (float)(srcRect.right - srcRect.left), (float)(srcRect.bottom - srcRect.top) });

        float midX = (clientMonitorinfo.rcWork.right - clientMonitorinfo.rcWork.left - (srcRect.right - srcRect.left)) / 2.f;
        float midY = (clientMonitorinfo.rcWork.bottom - clientMonitorinfo.rcWork.top - (srcRect.bottom - srcRect.top)) / 2.f;

        POINT lt = { srcRect.left, srcRect.top };
        ScreenToClient(clienthwnd, &lt);

        m_content.Offset({ (float)(lt.x - midX + g_widthOffset), (float)(lt.y - midY + g_heightOffset), 1.0f });
    }
    else
    {
        m_root.Size({ (float)(clientMonitorinfo.rcWork.right - clientMonitorinfo.rcWork.left), (float)(clientMonitorinfo.rcWork.bottom - clientMonitorinfo.rcWork.top) });
        m_content.Size(m_root.Size());
        m_content.Offset({ 0.0f, 0.0f, 0.0f });
    }  
}