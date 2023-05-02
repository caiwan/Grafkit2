#include <d3dcompiler.h>
#include <grafkit/core/Renderer.h>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

using namespace Grafkit::Core;

void Renderer::Shutdown()
{
	ID3D11Debug* d3dDebug = nullptr;
	if (SUCCEEDED(m_device->QueryInterface(__uuidof(ID3D11Debug), (void**)&d3dDebug)))
	{
		ID3D11InfoQueue* d3dInfoQueue = nullptr;
		if (SUCCEEDED(d3dDebug->QueryInterface(__uuidof(ID3D11InfoQueue), (void**)&d3dInfoQueue)))
		{
#ifdef _DEBUG
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_CORRUPTION, true);
			d3dInfoQueue->SetBreakOnSeverity(D3D11_MESSAGE_SEVERITY_ERROR, true);
#endif

			D3D11_MESSAGE_ID hide[] = {
				D3D11_MESSAGE_ID_SETPRIVATEDATA_CHANGINGPARAMS,
				// Add more message IDs here as needed
			};

			D3D11_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			d3dInfoQueue->AddStorageFilterEntries(&filter);
			d3dInfoQueue->Release();
		}
	}

	m_linearClampSampler->Release();
	m_linearClampSampler = nullptr;
	m_linearWrapSampler->Release();
	m_linearWrapSampler = nullptr;
	m_swapChain->Release();
	m_swapChain = nullptr;
	m_device->Release();
	m_device = nullptr;

	m_deviceContext->ClearState();
	m_deviceContext->Flush();
	m_deviceContext->Release();

#ifdef _DEBUG
	d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_SUMMARY | D3D11_RLDO_DETAIL);

	d3dDebug->Release();
#endif
}

ErrorCode Renderer::CreateDeviceAndSwapChainForHwnd(
	const HWND windowHandle,
	const size_t width,
	const size_t height,
	const RenderTarget& backBuffer) noexcept
{
	DXGI_SWAP_CHAIN_DESC SwapChainDesc {};
	SwapChainDesc.BufferCount = 1;
	SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	SwapChainDesc.BufferDesc.Width = width;
	SwapChainDesc.BufferDesc.Height = height;
	SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	SwapChainDesc.OutputWindow = windowHandle;
	SwapChainDesc.SampleDesc.Count = 4;
	SwapChainDesc.Windowed = true;
	SwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	UINT CreationFlags = 0;
#if defined(_DEBUG)
	CreationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	HRESULT HResult = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		CreationFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&SwapChainDesc,
		&m_swapChain,
		&m_device,
		nullptr,
		&m_deviceContext);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	ID3D11Texture2D* TemporaryBackBuffer = nullptr;
	HResult = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&TemporaryBackBuffer));
	if (HResult != S_OK)
	{
		TemporaryBackBuffer->Release();
		return ErrorCode::FAIL;
	}

	HResult = m_device->CreateRenderTargetView(TemporaryBackBuffer, nullptr, &backBuffer.renderTargetView);
	TemporaryBackBuffer->Release();
	TemporaryBackBuffer = nullptr;
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	backBuffer.width = width;
	backBuffer.height = height;

	// create clamp sampler
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MinLOD = -FLT_MAX;
	samplerDesc.MaxLOD = FLT_MAX;

	HResult = m_device->CreateSamplerState(&samplerDesc, &m_linearClampSampler);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	// create wrap sampler
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	HResult = m_device->CreateSamplerState(&samplerDesc, &m_linearWrapSampler);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	return ErrorCode::OK;
}

ErrorCode Renderer::CreateIndexBufferWithData(const uint32_t* Data, const size_t Count, Buffer& Buffer) const noexcept
{
	D3D11_BUFFER_DESC BufferDesc {};
	BufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
	BufferDesc.ByteWidth = sizeof(uint32_t) * Count;
	BufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;

	Buffer.stride = sizeof(uint32_t);

	D3D11_SUBRESOURCE_DATA InitialData {};
	InitialData.pSysMem = Data;
	InitialData.SysMemPitch = 0;
	InitialData.SysMemSlicePitch = 0;

	const auto hresult = m_device->CreateBuffer(&BufferDesc, &InitialData, &Buffer.buffer);
	if (hresult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	return ErrorCode::OK;
}

ErrorCode Renderer::CreateVertexShader(
	const wchar_t* FileName,
	const char* EntryPoint,
	const D3D11_INPUT_ELEMENT_DESC* InputElementDescriptorArray,
	const size_t InputElementCount,
	Shader& Shader) const noexcept
{
	ID3DBlob* Blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	auto HResult = D3DCompileFromFile(FileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint, "vs_5_0", flags, 0, &Blob, nullptr);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	HResult = m_device->CreateVertexShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, &Shader.vertex);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}
	Shader.vertex->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(FileName) - 1, FileName);
	if (InputElementDescriptorArray)
	{
		HResult = m_device->CreateInputLayout(
			InputElementDescriptorArray, InputElementCount, Blob->GetBufferPointer(), Blob->GetBufferSize(), &Shader.layout);
		if (HResult != S_OK)
		{
			return ErrorCode::FAIL;
		}
	}
	Shader.stage |= ShaderStage::VERTEX;
	Blob->Release();
	return ErrorCode::OK;
}

ErrorCode Renderer::CreatePixelShader(const wchar_t* FileName, const char* EntryPoint, Shader& Shader) const noexcept
{
	ID3DBlob* Blob;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined(DEBUG) || defined(_DEBUG)
	flags |= D3DCOMPILE_DEBUG;
#endif

	auto HResult = D3DCompileFromFile(FileName, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, EntryPoint, "ps_5_0", flags, 0, &Blob, nullptr);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	HResult = m_device->CreatePixelShader(Blob->GetBufferPointer(), Blob->GetBufferSize(), nullptr, &Shader.pixel);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}
	Shader.pixel->SetPrivateData(WKPDID_D3DDebugObjectName, sizeof(FileName) - 1, FileName);
	Shader.stage |= ShaderStage::PIXEL;
	Blob->Release();
	return ErrorCode::OK;
}

ErrorCode Renderer::CreateRenderTarget(const uint32_t Width, const uint32_t Height, const DXGI_FORMAT Format, RenderTarget& RenderTarget)
	const noexcept
{
	D3D11_TEXTURE2D_DESC TextureDesc {};
	TextureDesc.Width = Width;
	TextureDesc.Height = Height;
	TextureDesc.MipLevels = 1;
	TextureDesc.ArraySize = 1;
	TextureDesc.Format = Format;
	TextureDesc.SampleDesc.Count = 1;
	TextureDesc.Usage = D3D11_USAGE_DEFAULT;
	TextureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	TextureDesc.CPUAccessFlags = 0;
	TextureDesc.MiscFlags = 0;
	auto HResult = m_device->CreateTexture2D(&TextureDesc, nullptr, &RenderTarget.texture);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	D3D11_RENDER_TARGET_VIEW_DESC RenderTargetViewDesc {};
	RenderTargetViewDesc.Format = TextureDesc.Format;
	RenderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	RenderTargetViewDesc.Texture2D.MipSlice = 0;
	HResult = m_device->CreateRenderTargetView(RenderTarget.texture, &RenderTargetViewDesc, &RenderTarget.renderTargetView);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC ShaderResourceViewDesc {};
	ShaderResourceViewDesc.Format = TextureDesc.Format;
	ShaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	ShaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	ShaderResourceViewDesc.Texture2D.MipLevels = 1;
	HResult = m_device->CreateShaderResourceView(RenderTarget.texture, &ShaderResourceViewDesc, &RenderTarget.shaderResourceView);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	RenderTarget.width = Width;
	RenderTarget.height = Height;

	return ErrorCode::OK;
}

ErrorCode
Renderer::CreateDepthStencil(const uint32_t Width, const uint32_t Height, const DXGI_FORMAT Format, const RenderTarget& DepthStencil)
	const noexcept
{
	D3D11_TEXTURE2D_DESC DepthStencilDesc {};
	DepthStencilDesc.Width = Width;
	DepthStencilDesc.Height = Height;
	DepthStencilDesc.MipLevels = 1;
	DepthStencilDesc.ArraySize = 1;
	DepthStencilDesc.Format = Format;
	DepthStencilDesc.SampleDesc.Count = 1;
	DepthStencilDesc.SampleDesc.Quality = 0;
	DepthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
	DepthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	DepthStencilDesc.CPUAccessFlags = 0;
	DepthStencilDesc.MiscFlags = 0;

	auto HResult = m_device->CreateTexture2D(&DepthStencilDesc, nullptr, &DepthStencil.depthTexture);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}
	HResult = m_device->CreateDepthStencilView(DepthStencil.depthTexture, nullptr, &DepthStencil.depthStencilView);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}
	return ErrorCode::OK;
}

ErrorCode Renderer::CreateTextureFromFile(const char* FileName, const DXGI_FORMAT Format, RenderTarget& Texture) const noexcept
{
	int ImageWidth = 0;
	int ImageHeight = 0;
	int Components = 0;
	uint8_t* ImageData = stbi_load(FileName, &ImageWidth, &ImageHeight, &Components, 4);
	if (ImageData == nullptr)
	{
		return ErrorCode::FAIL;
	}

	const auto Error = CreateTextureFromMemory(ImageData, ImageWidth, ImageHeight, 4, Format, Texture);
	if (Error != ErrorCode::OK)
	{
		return Error;
	}
	Texture.width = ImageWidth;
	Texture.height = ImageHeight;
	stbi_image_free(ImageData);

	return ErrorCode::OK;
}

ErrorCode Renderer::CreateTextureFromMemory(
	const uint8_t* Data,
	const uint32_t Width,
	const uint32_t Height,
	const uint8_t Components,
	const DXGI_FORMAT Format,
	RenderTarget& Texture) const noexcept
{
	D3D11_TEXTURE2D_DESC TextureDescriptor {};
	TextureDescriptor.Width = Width;
	TextureDescriptor.Height = Height;
	TextureDescriptor.MipLevels = 1;
	TextureDescriptor.ArraySize = 1;
	TextureDescriptor.Format = Format;
	TextureDescriptor.SampleDesc.Count = 1;
	TextureDescriptor.Usage = D3D11_USAGE_IMMUTABLE;
	TextureDescriptor.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	TextureDescriptor.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA Subresource {};
	Subresource.pSysMem = Data;
	Subresource.SysMemPitch = TextureDescriptor.Width * Components;
	Subresource.SysMemSlicePitch = 0;
	auto HResult = m_device->CreateTexture2D(&TextureDescriptor, &Subresource, &Texture.texture);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc {};
	srvDesc.Format = Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = TextureDescriptor.MipLevels;
	srvDesc.Texture2D.MostDetailedMip = 0;
	m_device->CreateShaderResourceView(Texture.texture, &srvDesc, &Texture.shaderResourceView);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}

	return ErrorCode::OK;
}

ErrorCode Renderer::CreateCubeMapTexture(const char* Directory, RenderTarget& CubeMap) const noexcept
{

	struct CubeImageData
	{
		int ImageWidth = 0;
		int ImageHeight = 0;
		int Components = 0;
		std::string FileName;
		uint8_t* Pixels = nullptr;
	};

	CubeImageData images[6];

	images[0].FileName = Directory;
	images[0].FileName += "nx.png";

	images[1].FileName = Directory;
	images[1].FileName += "ny.png";

	images[2].FileName = Directory;
	images[2].FileName += "nz.png";

	images[3].FileName = Directory;
	images[3].FileName += "px.png";

	images[4].FileName = Directory;
	images[4].FileName += "py.png";

	images[5].FileName = Directory;
	images[5].FileName += "pz.png";

	images[0].Pixels = stbi_load(images[0].FileName.c_str(), &images[0].ImageWidth, &images[0].ImageHeight, &images[0].Components, 4);
	images[1].Pixels = stbi_load(images[1].FileName.c_str(), &images[1].ImageWidth, &images[1].ImageHeight, &images[1].Components, 4);
	images[2].Pixels = stbi_load(images[2].FileName.c_str(), &images[2].ImageWidth, &images[2].ImageHeight, &images[2].Components, 4);
	images[3].Pixels = stbi_load(images[3].FileName.c_str(), &images[3].ImageWidth, &images[3].ImageHeight, &images[3].Components, 4);
	images[4].Pixels = stbi_load(images[4].FileName.c_str(), &images[4].ImageWidth, &images[4].ImageHeight, &images[4].Components, 4);
	images[5].Pixels = stbi_load(images[5].FileName.c_str(), &images[5].ImageWidth, &images[5].ImageHeight, &images[5].Components, 4);

	for (size_t Index = 0; Index < 6; ++Index)
	{
		if (images[Index].Pixels == nullptr)
		{
			return ErrorCode::FAIL;
		}
	}

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = 2048;
	textureDesc.Height = 2048;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 6;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_TEXTURECUBE;

	D3D11_SHADER_RESOURCE_VIEW_DESC SMViewDesc;
	SMViewDesc.Format = textureDesc.Format;
	SMViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURECUBE;
	SMViewDesc.TextureCube.MipLevels = textureDesc.MipLevels;
	SMViewDesc.TextureCube.MostDetailedMip = 0;

	D3D11_SUBRESOURCE_DATA pData[6];
	for (size_t Index = 0; Index < 6; ++Index)
	{
		pData[Index].pSysMem = images[Index].Pixels;
		pData[Index].SysMemPitch = images[Index].Components;
		pData[Index].SysMemSlicePitch = 0;
	}

	HRESULT HResult = m_device->CreateTexture2D(&textureDesc, &pData[0], &CubeMap.texture);
	if (HResult != S_OK)
	{
		stbi_image_free(images[0].Pixels);
		stbi_image_free(images[1].Pixels);
		stbi_image_free(images[2].Pixels);
		stbi_image_free(images[3].Pixels);
		stbi_image_free(images[4].Pixels);
		stbi_image_free(images[5].Pixels);
		return ErrorCode::FAIL;
	}
	HResult = m_device->CreateShaderResourceView(CubeMap.texture, &SMViewDesc, &CubeMap.shaderResourceView);
	if (HResult != S_OK)
	{
		stbi_image_free(images[0].Pixels);
		stbi_image_free(images[1].Pixels);
		stbi_image_free(images[2].Pixels);
		stbi_image_free(images[3].Pixels);
		stbi_image_free(images[4].Pixels);
		stbi_image_free(images[5].Pixels);
		return ErrorCode::FAIL;
	}

	stbi_image_free(images[0].Pixels);
	stbi_image_free(images[1].Pixels);
	stbi_image_free(images[2].Pixels);
	stbi_image_free(images[3].Pixels);
	stbi_image_free(images[4].Pixels);
	stbi_image_free(images[5].Pixels);

	return ErrorCode::OK;
}

void Renderer::DestroyRenderTarget(RenderTarget& RenderTarget) const noexcept
{
	if (RenderTarget.renderTargetView != nullptr)
	{
		RenderTarget.renderTargetView->Release();
		RenderTarget.renderTargetView = nullptr;
	}
	if (RenderTarget.shaderResourceView != nullptr)
	{
		RenderTarget.shaderResourceView->Release();
		RenderTarget.shaderResourceView = nullptr;
	}
	if (RenderTarget.depthStencilView != nullptr)
	{
		RenderTarget.depthStencilView->Release();
		RenderTarget.depthStencilView = nullptr;
	}
	if (RenderTarget.texture != nullptr)
	{
		RenderTarget.texture->Release();
		RenderTarget.texture = nullptr;
	}
	if (RenderTarget.depthTexture != nullptr)
	{
		RenderTarget.depthTexture->Release();
		RenderTarget.depthTexture = nullptr;
	}
}

void Renderer::DestroyTexture(RenderTarget& Texture) const noexcept
{
	if (Texture.shaderResourceView != nullptr)
	{
		Texture.shaderResourceView->Release();
		Texture.shaderResourceView = nullptr;
	}
	if (Texture.texture != nullptr)
	{
		Texture.texture->Release();
		Texture.texture = nullptr;
	}
}

void Renderer::DestroyBuffer(Buffer& Buffer) const noexcept
{
	if (Buffer.buffer)
	{
		Buffer.buffer->Release();
	}
}

void Renderer::DestroyShader(Shader& Shader) const noexcept
{
	if (Shader.vertex)
	{
		Shader.vertex->Release();
	}
	if (Shader.pixel)
	{
		Shader.pixel->Release();
	}
	if (Shader.layout)
	{
		Shader.layout->Release();
	}
}

void Renderer::ResizeBackBuffer(const uint32_t Width, const uint32_t Height, const RenderTarget& BackBuffer) const noexcept
{
	if (!m_device || !m_swapChain)
	{
		return;
	}
	if (BackBuffer.renderTargetView)
	{
		BackBuffer.renderTargetView->Release();
		BackBuffer.renderTargetView = nullptr;
	}

	m_swapChain->ResizeBuffers(0, Width, Height, DXGI_FORMAT_UNKNOWN, 0);
	ID3D11Texture2D* TemporaryBackBuffer;
	m_swapChain->GetBuffer(0, IID_PPV_ARGS(&TemporaryBackBuffer));
	m_device->CreateRenderTargetView(TemporaryBackBuffer, nullptr, &BackBuffer.renderTargetView);
	TemporaryBackBuffer->Release();

	BackBuffer.width = Width;
	BackBuffer.height = Height;
}

void Renderer::ClearRenderTarget(const RenderTarget& RenderTarget, const DirectX::XMFLOAT4& Colour) const noexcept
{
	m_deviceContext->ClearRenderTargetView(RenderTarget.renderTargetView, &Colour.x);
}

void Renderer::ClearDepthStencil(const RenderTarget& RenderTarget, const uint32_t ClearFlags, const float Depth, const uint8_t Stencil)
	const noexcept
{
	m_deviceContext->ClearDepthStencilView(RenderTarget.depthStencilView, ClearFlags, Depth, Stencil);
}

void Renderer::UnbindRenderTargets() const noexcept
{
	ID3D11ShaderResourceView* NullSRViews[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
												nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };
	ID3D11RenderTargetView* NullRTViews[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

	m_deviceContext->PSSetShaderResources(0, 16, NullSRViews);
	m_deviceContext->OMSetRenderTargets(8, NullRTViews, nullptr);
}

void Renderer::SetConstantBuffer(const Buffer& ConstantBuffer, const ShaderStage ShaderStage, const size_t Slot) const noexcept
{
	if ((ShaderStage & ShaderStage::VERTEX) == ShaderStage::VERTEX)
	{
		m_deviceContext->VSSetConstantBuffers(Slot, 1, &ConstantBuffer.buffer);
	}

	if ((ShaderStage & ShaderStage::PIXEL) == ShaderStage::PIXEL)
	{
		m_deviceContext->PSSetConstantBuffers(Slot, 1, &ConstantBuffer.buffer);
	}
}

void Renderer::SetViewport(
	const uint32_t Width,
	const uint32_t Height,
	const uint32_t XOffset,
	const uint32_t YOffset,
	const float MinDepth,
	const float MaxDepth) const noexcept
{
	D3D11_VIEWPORT Viewport {};
	Viewport.Width = static_cast<FLOAT>(Width);
	Viewport.Height = static_cast<FLOAT>(Height);
	Viewport.TopLeftX = static_cast<FLOAT>(XOffset);
	Viewport.TopLeftY = static_cast<FLOAT>(YOffset);
	Viewport.MinDepth = MinDepth;
	Viewport.MaxDepth = MaxDepth;

	m_deviceContext->RSSetViewports(1, &Viewport);
}

void Renderer::SetShader(const Shader& shader) const noexcept
{
	if ((shader.stage & ShaderStage::VERTEX) == ShaderStage::VERTEX)
	{
		m_deviceContext->VSSetShader(shader.vertex, nullptr, 0);
		m_deviceContext->IASetInputLayout(shader.layout);
	}

	if ((shader.stage & ShaderStage::PIXEL) == ShaderStage::PIXEL)
	{
		m_deviceContext->PSSetShader(shader.pixel, nullptr, 0);
	}
}

void Renderer::SetRenderTarget(const RenderTarget& renderTarget) const noexcept
{
	m_deviceContext->OMSetRenderTargets(1, &renderTarget.renderTargetView, renderTarget.depthStencilView);
}

void Renderer::SetRenderTargets(const size_t count, const RenderTarget* renderTarget) const noexcept
{
	ID3D11RenderTargetView* RenderTargetViewArray[D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT];

	for (size_t Index = 0; Index < count; ++Index)
	{
		RenderTargetViewArray[Index] = renderTarget[Index].renderTargetView;
	}

	m_deviceContext->OMSetRenderTargets(count, RenderTargetViewArray, renderTarget[0].depthStencilView);
}

void Renderer::SetTexture(const uint32_t slot, const RenderTarget& texture) const noexcept
{
	ID3D11SamplerState* samplers[] = { m_linearClampSampler, m_linearWrapSampler };
	m_deviceContext->PSSetShaderResources(slot, 1, &texture.shaderResourceView);
	m_deviceContext->PSSetSamplers(0, 2, samplers);
}

void Renderer::SetPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology) const noexcept
{
	m_deviceContext->IASetPrimitiveTopology(PrimitiveTopology);
}

void Renderer::SetVertexBuffer(const size_t StartSlot, const Buffer& Buffer, const uint32_t Offset) const noexcept
{
	m_deviceContext->IASetVertexBuffers(StartSlot, 1, &Buffer.buffer, &Buffer.stride, &Offset);
}

void Renderer::SetIndexBuffer(const size_t StartSlot, const Buffer& Buffer, const uint32_t Offset) const noexcept
{
	m_deviceContext->IASetIndexBuffer(Buffer.buffer, DXGI_FORMAT_R32_UINT, Offset);
}

void Renderer::Draw(const size_t vertexCount, const size_t vertexLocationStart) const noexcept
{
	m_deviceContext->Draw(vertexCount, vertexLocationStart);
}

void Renderer::DrawIndexed(const size_t indexCount, const size_t indexLocationStart, const size_t vertexLocationBase) const noexcept
{
	m_deviceContext->DrawIndexed(indexCount, indexLocationStart, vertexLocationBase);
}

ErrorCode Renderer::Present(const size_t syncInterval, const size_t flags) const noexcept
{
	const auto HResult = m_swapChain->Present(syncInterval, flags);
	if (HResult != S_OK)
	{
		return ErrorCode::FAIL;
	}
	return ErrorCode::OK;
}
