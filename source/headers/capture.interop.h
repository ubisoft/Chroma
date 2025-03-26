#pragma once
#include <winrt/Windows.Graphics.Capture.h>
#include <windows.graphics.capture.interop.h>
#include <windows.graphics.capture.h>

inline auto CreateCaptureItemForWindow(HWND hwnd)
{
	winrt::Windows::Graphics::Capture::GraphicsCaptureItem item = { nullptr };
	HRESULT hr;
	if(hwnd)
	{
		try 
		{
			auto activation_factory = winrt::get_activation_factory<winrt::Windows::Graphics::Capture::GraphicsCaptureItem>();
			auto interop_factory = activation_factory.as<IGraphicsCaptureItemInterop>();
			hr = interop_factory->CreateForWindow(hwnd, winrt::guid_of<ABI::Windows::Graphics::Capture::IGraphicsCaptureItem>(), reinterpret_cast<void**>(winrt::put_abi(item)));
		}
		catch (.../*winrt::hresult_error const& ex*/) 
		{
			/*MessageBoxW(hwnd, ex.message().c_str(), NULL, MB_ABORTRETRYIGNORE);*/
		}
	}
	return item;
}