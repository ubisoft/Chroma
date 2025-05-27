#include "../headers/pch.h"
#include "../headers/SimpleCapture.h"
#include <windows.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <sal.h>
#include <new>
#include <warning.h>
#include <DirectXMath.h>
#include "../shaders/DefaultPixelShader.h"
#include "../shaders/ProtanPixelShader.h"
#include "../shaders/DeutanPixelShader.h"
#include "../shaders/TritanPixelShader.h"
#include "../shaders/GreyscalePixelShader.h"
#include "../headers/App.h"

using namespace winrt;
using namespace Windows::Foundation;
using namespace Windows::System;
using namespace Windows::Graphics::Capture;
using namespace Windows::Graphics::DirectX;
using namespace Windows::Graphics::DirectX::Direct3D11;
using namespace Windows::Foundation::Numerics;
using namespace Windows::UI;
using namespace Windows::UI::Composition;

using namespace DirectX;

extern std::map<int, std::string> globalList;
extern INT g_appliedMode;

typedef struct _VERTEX
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT2 TexCoord;
} VERTEX;

SimpleCapture::SimpleCapture(
	IDirect3DDevice const& device,
	GraphicsCaptureItem const& item,
	HWND const& clientHwnd,
	int const& filter)
{
	m_item = item;
	m_device = device;
	m_clientHwnd = clientHwnd;
	m_filter = filter;
	auto itemSize = m_item.Size();

	m_3dDevice = GetDXGIInterfaceFromObject<ID3D11Device>(m_device);
	m_3dDevice->GetImmediateContext(m_d3dContext.put());

	// Get DXGI factory
	IDXGIDevice* DxgiDevice = nullptr;
	HRESULT hr  = m_3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&DxgiDevice));
	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to query IDXGIDevice.");
	}

	IDXGIAdapter* DxgiAdapter = nullptr;
	hr = DxgiDevice->GetParent(__uuidof(IDXGIAdapter), reinterpret_cast<void**>(&DxgiAdapter));
	DxgiDevice->Release();
	DxgiDevice = nullptr;
	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to get parent IDXGIAdapter.");
	}

	hr = DxgiAdapter->GetParent(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&m_Factory));
	DxgiAdapter->Release();
	DxgiAdapter = nullptr;
	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to get parent IDXGIFactory2.");
	}
	
	// Get window size
	UINT Width = static_cast<uint32_t>(itemSize.Width);
	UINT Height = static_cast<uint32_t>(itemSize.Height);

	// Create swapchain for window
	DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
	RtlZeroMemory(&SwapChainDesc, sizeof(SwapChainDesc));
	
	SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	SwapChainDesc.BufferCount = 2;
	SwapChainDesc.Width = Width;
	SwapChainDesc.Height = Height;
	SwapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.SampleDesc.Count = 1;
	SwapChainDesc.SampleDesc.Quality = 0;
	hr = m_Factory->CreateSwapChainForComposition(m_3dDevice.get(), &SwapChainDesc, nullptr, &m_SwapChain);

	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to create swap chain for composition.");
	}

	// Disable the ALT-ENTER shortcut for entering full-screen mode
	hr = m_Factory->MakeWindowAssociation(m_clientHwnd, DXGI_MWA_NO_ALT_ENTER);
	if (FAILED(hr))
	{
		// This is not always a critical failure, but for consistency with previous asserts:
		throw winrt::hresult_error(hr, L"Failed to make window association.");
	}	
	// Create shared texture
	hr = CreateSharedSurf();
	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to create shared surface.");
	}
	// Make new render target view
	hr = MakeRTV();
	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to make render target view.");
	}
	// Set view port
	hr = SetViewPort(Width, Height);
	if (FAILED(hr))
	{
		// SetViewPort currently always returns S_OK, but for robustness:
		throw winrt::hresult_error(hr, L"Failed to set view port.");
	}
	// Create the sample state
	D3D11_SAMPLER_DESC SampDesc;
	RtlZeroMemory(&SampDesc, sizeof(SampDesc));
	SampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	SampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	SampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	SampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	SampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	SampDesc.MinLOD = 0;
	SampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	hr = m_3dDevice->CreateSamplerState(&SampDesc, &m_SamplerLinear);
	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to create sampler state.");
	}
	
	// Create the blend state
	D3D11_BLEND_DESC BlendStateDesc;
	BlendStateDesc.AlphaToCoverageEnable = FALSE;
	BlendStateDesc.IndependentBlendEnable = FALSE;
	BlendStateDesc.RenderTarget[0].BlendEnable = TRUE;
	BlendStateDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	BlendStateDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	BlendStateDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	BlendStateDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	BlendStateDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	BlendStateDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	BlendStateDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
	hr = m_3dDevice->CreateBlendState(&BlendStateDesc, &m_BlendState);
	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to create blend state.");
	}

	// Initialize shaders
	hr = InitShaders();
	if (FAILED(hr))
	{
		throw winrt::hresult_error(hr, L"Failed to initialize shaders.");
	}
	// END OF ADDED CHANGES
	auto size = m_item.Size();

	// Create framepool, define pixel format (DXGI_FORMAT_B8G8R8A8_UNORM), and frame size. 
	m_framePool = Direct3D11CaptureFramePool::Create(
		m_device,
		DirectXPixelFormat::B8G8R8A8UIntNormalized,
		2,
		size);
	m_session = m_framePool.CreateCaptureSession(m_item);
	m_lastSize = size;
	m_frameArrived = m_framePool.FrameArrived(auto_revoke, { this, &SimpleCapture::OnFrameArrived });

	current_ticks = std::chrono::high_resolution_clock::now();

	m_windowText = "";

	int Length = GetWindowTextLength(m_clientHwnd);
	char* charwindowText = new char[(int)(Length + 1)];
	GetWindowTextA(m_clientHwnd, charwindowText, (Length + 1));
	m_windowText = charwindowText + std::string(" : ");
	delete charwindowText;
}

//
// Initialize shaders for drawing to screen
//
HRESULT SimpleCapture::InitShaders()
{
	HRESULT hr;

	UINT Size = ARRAYSIZE(g_VS1);
	hr = m_3dDevice->CreateVertexShader(g_VS1, Size, nullptr, &m_VertexShader);
	if (FAILED(hr))
	{
		//assert(false); // Removed assert
		return hr;
	}

	D3D11_INPUT_ELEMENT_DESC Layout[] =
	{
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
	};
	UINT NumElements = ARRAYSIZE(Layout);

	hr = m_3dDevice->CreateInputLayout(Layout, NumElements, g_VS1, Size, &m_InputLayout);
	if (FAILED(hr))
	{
		//assert(false); // Removed assert
		return hr;
	}

	m_d3dContext->IASetInputLayout(m_InputLayout);
	switch (m_filter)
	{
	case ID_FILTER_DEFAULT:
		Size = ARRAYSIZE(g_defaultMain);
		hr = m_3dDevice->CreatePixelShader(g_defaultMain, Size, nullptr, &m_PixelShader);
		if (FAILED(hr))
		{
			//assert(false); // Removed assert
			return hr;
		}
		break;
	case ID_FILTER_PROTAN:
		Size = ARRAYSIZE(g_protanMain);
		hr = m_3dDevice->CreatePixelShader(g_protanMain, Size, nullptr, &m_PixelShader);
		if (FAILED(hr))
		{
			//assert(false); // Removed assert
			return hr;
		}
		break;
	case ID_FILTER_DEUTAN:
		Size = ARRAYSIZE(g_deutanMain);
		hr = m_3dDevice->CreatePixelShader(g_deutanMain, Size, nullptr, &m_PixelShader);
		if (FAILED(hr))
		{
			//assert(false); // Removed assert
			return hr;
		}
		break;
	case ID_FILTER_TRITAN:
		Size = ARRAYSIZE(g_tritanMain);
		hr = m_3dDevice->CreatePixelShader(g_tritanMain, Size, nullptr, &m_PixelShader);
		if (FAILED(hr))
		{
			//assert(false); // Removed assert
			return hr;
		}
		break;
	case ID_FILTER_GSCALE:
		Size = ARRAYSIZE(g_greyscaleMain);
		hr = m_3dDevice->CreatePixelShader(g_greyscaleMain, Size, nullptr, &m_PixelShader);
		if (FAILED(hr))
		{
			//assert(false); // Removed assert
			return hr;
		}
		break;
	default:
		Size = ARRAYSIZE(g_defaultMain);
		hr = m_3dDevice->CreatePixelShader(g_defaultMain, Size, nullptr, &m_PixelShader);
		if (FAILED(hr))
		{
			//assert(false); // Removed assert
			return hr;
		}
	}
	return S_OK;
}

HRESULT SimpleCapture::SetViewPort(UINT Width, UINT Height)
{
	D3D11_VIEWPORT VP;
	VP.Width = static_cast<FLOAT>(Width);
	VP.Height = static_cast<FLOAT>(Height);
	VP.MinDepth = 0.0f;
	VP.MaxDepth = 1.0f;
	VP.TopLeftX = 0;
	VP.TopLeftY = 0;
	m_d3dContext->RSSetViewports(1, &VP);
	return S_OK;
}

//
// Reset render target view
//
HRESULT SimpleCapture::MakeRTV()
{
	// Get backbuffer
	ID3D11Texture2D* BackBuffer = nullptr;
	HRESULT hr = m_SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&BackBuffer));
	if (FAILED(hr))
	{
		//assert(false); // Removed assert
		return hr;
	}
	// Create a render target view
	hr = m_3dDevice->CreateRenderTargetView(BackBuffer, nullptr, &m_RTV);
	BackBuffer->Release();
	if (FAILED(hr))
	{
		//assert(false); // Removed assert
		return hr;
	}
	// Set new render target
	m_d3dContext->OMSetRenderTargets(1, &m_RTV, nullptr);

	return S_OK;
}

//
// Recreate shared texture
//
HRESULT SimpleCapture::CreateSharedSurf()
{
	auto size = m_item.Size();

	// Create shared texture for all duplication threads to draw into
	D3D11_TEXTURE2D_DESC DeskTexD;
	RtlZeroMemory(&DeskTexD, sizeof(D3D11_TEXTURE2D_DESC));
	DeskTexD.Width = static_cast<uint32_t>(size.Width);
	DeskTexD.Height = static_cast<uint32_t>(size.Height);
	DeskTexD.MipLevels = 1;
	DeskTexD.ArraySize = 1;
	DeskTexD.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	DeskTexD.SampleDesc.Count = 1;
	DeskTexD.Usage = D3D11_USAGE_DEFAULT;
	DeskTexD.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	DeskTexD.CPUAccessFlags = 0;
	DeskTexD.MiscFlags = 0;

	HRESULT hr = m_3dDevice->CreateTexture2D(&DeskTexD, nullptr, &m_SharedSurf);
	if (FAILED(hr))
	{
		//assert(false); // Removed assert
		return hr;
	}   
	return S_OK;
}

// Start sending capture frames
void SimpleCapture::StartCapture()
{
	CheckClosed();
	m_session.StartCapture();
}

ICompositionSurface SimpleCapture::CreateSurface(Compositor const& compositor)
{
	CheckClosed();
	return CreateCompositionSurfaceForSwapChain(compositor, m_SwapChain);
}

void SimpleCapture::CheckClosed()
{
	if (SimpleCapture::m_closed.load() == true)
	{
		throw winrt::hresult_error(RO_E_CLOSED);
	}
}

// Process captured frames
void SimpleCapture::Close()
{
	auto expected = false;
	if (SimpleCapture::m_closed.compare_exchange_strong(expected, true))
	{
		if (m_session) 
		{
			m_session.Close();
			m_session = nullptr;
		}
		if (m_VertexShader) 
		{
			m_VertexShader->Release(); 
			m_VertexShader = nullptr;
		}
		if (m_frameArrived) { m_frameArrived.revoke(); }
		if (m_framePool) 
		{
			m_framePool.Close(); 
			m_framePool = nullptr;
		}
		if (m_d3dContext) 
		{
			m_d3dContext->Flush();
			m_d3dContext = nullptr;
		}		
		if (m_SwapChain) 
		{ 
			m_SwapChain->Release(); 
			m_SwapChain = nullptr;
		}
		if (m_InputLayout) 
		{
			m_InputLayout->Release(); 
			m_InputLayout = nullptr;
		}
		if (m_PixelShader) 
		{
			m_PixelShader->Release(); 
			m_PixelShader = nullptr;
		}
		if (m_BlendState) 
		{
			m_BlendState->Release();
			m_BlendState = nullptr;
		}
		if (m_RTV) 
		{ 
			m_RTV->Release(); 
			m_RTV = nullptr;
		}
		if (m_SamplerLinear) 
		{ 
			m_SamplerLinear->Release();
			m_SamplerLinear = nullptr;
		}
		if (m_SharedSurf) 
		{ 
			m_SharedSurf->Release(); 
			m_SharedSurf = nullptr;
		}

		m_item = nullptr;
		m_device = nullptr;
		m_Factory = nullptr;	
	}
}

void SimpleCapture::OnFrameArrived(
	Direct3D11CaptureFramePool const& sender,
	winrt::Windows::Foundation::IInspectable const&)
{	
	auto newSize = false;

	auto frame = sender.TryGetNextFrame();
	auto frameContentSize = frame.ContentSize();
	auto frameSurface = GetDXGIInterfaceFromObject<ID3D11Texture2D>(frame.Surface());
	m_d3dContext->CopyResource(m_SharedSurf, frameSurface.get());

	// Vertices for drawing whole texture
	VERTEX Vertices[NUMVERTICES] =
	{
		{XMFLOAT3(-1.0f, -1.0f, 0), XMFLOAT2(0.0f, 1.0f)},
		{XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(1.0f, -1.0f, 0), XMFLOAT2(1.0f, 1.0f)},
		{XMFLOAT3(-1.0f, 1.0f, 0), XMFLOAT2(0.0f, 0.0f)},
		{XMFLOAT3(1.0f, 1.0f, 0), XMFLOAT2(1.0f, 0.0f)},
	};

	D3D11_TEXTURE2D_DESC FrameDesc;
	m_SharedSurf->GetDesc(&FrameDesc);

	D3D11_SHADER_RESOURCE_VIEW_DESC ShaderDesc;
	ShaderDesc.Format = FrameDesc.Format;
	ShaderDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ShaderDesc.Texture2D.MostDetailedMip = FrameDesc.MipLevels - 1;
	ShaderDesc.Texture2D.MipLevels = FrameDesc.MipLevels;

	// Create new shader resource view
	ID3D11ShaderResourceView* ShaderResource = nullptr;
		
	HRESULT hr = m_3dDevice->CreateShaderResourceView(m_SharedSurf, &ShaderDesc, &ShaderResource);
	if (FAILED(hr))
	{
		// assert(false); // Removed assert from OnFrameArrived
		// This error is in OnFrameArrived. For now, just return to skip the frame.
		// A more robust solution might involve trying to recover or stopping capture.
		if (ShaderResource) { ShaderResource->Release(); ShaderResource = nullptr; }
		return;
	}

	// Set resources
	UINT Stride = sizeof(VERTEX);
	UINT Offset = 0;
	FLOAT blendFactor[4] = { 0.f, 0.f, 0.f, 0.f };
	
	m_d3dContext->OMSetBlendState(nullptr, blendFactor, 0xffffffff);
	m_d3dContext->OMSetRenderTargets(1, &m_RTV, nullptr);
	m_d3dContext->VSSetShader(m_VertexShader, nullptr, 0);
	m_d3dContext->PSSetShader(m_PixelShader, nullptr, 0);
	m_d3dContext->PSSetShaderResources(0, 1, &ShaderResource);
	m_d3dContext->PSSetSamplers(0, 1, &m_SamplerLinear);
	m_d3dContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	
	D3D11_BUFFER_DESC BufferDesc;
	RtlZeroMemory(&BufferDesc, sizeof(BufferDesc));
	BufferDesc.Usage = D3D11_USAGE_DEFAULT;
	BufferDesc.ByteWidth = sizeof(VERTEX) * NUMVERTICES;
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.CPUAccessFlags = 0;
	D3D11_SUBRESOURCE_DATA InitData;
	RtlZeroMemory(&InitData, sizeof(InitData));
	InitData.pSysMem = Vertices;    
		
	ID3D11Buffer* VertexBuffer = nullptr;

	// Create vertex buffer
	hr = m_3dDevice->CreateBuffer(&BufferDesc, &InitData, &VertexBuffer);
	if (FAILED(hr))
	{
		// assert(false); // Removed assert from OnFrameArrived
		// This error is in OnFrameArrived. For now, just return to skip the frame.
		if (ShaderResource) { ShaderResource->Release(); ShaderResource = nullptr; }
		if (VertexBuffer) { VertexBuffer->Release(); VertexBuffer = nullptr; }
		return;
	}
	m_d3dContext->IASetVertexBuffers(0, 1, &VertexBuffer, &Stride, &Offset);

	frame.Close();
	m_d3dContext->Draw(NUMVERTICES, 0);
	m_d3dContext->Flush();
	VertexBuffer->Release();
	VertexBuffer = nullptr;

	// Release shader resource
	ShaderResource->Release();
	ShaderResource = nullptr;
 
	DXGI_PRESENT_PARAMETERS presentParameters = { 0 };
	m_SwapChain->Present1(1, 0, &presentParameters);

	if (newSize)
	{
		m_framePool.Recreate(
			m_device,
			DirectXPixelFormat::B8G8R8A8UIntNormalized,
			2,
			m_lastSize);
	}

	last_ticks = std::chrono::high_resolution_clock::now();

	std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(last_ticks-current_ticks);

	if (counter > 50)
	{
		long finalFps = FPSTotal / counter;

		SetWindowTextA(m_clientHwnd, "");
		SetWindowTextA(m_clientHwnd, std::string(globalList[g_appliedMode] + " : " + m_windowText + std::string(std::to_string(finalFps))).c_str());
		counter = 0;
		FPSTotal = 0;
	}

	double count = time_span.count() * 1000;
	clock_t fps = 0;
	if (count > 0)
	{
		fps = (clock_t)(CLOCKS_PER_SEC / count);
	}

	FPSTotal = FPSTotal + fps;
	counter++;
	current_ticks = std::chrono::high_resolution_clock::now();
}