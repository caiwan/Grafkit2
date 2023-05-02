#pragma once

#include <d3d11.h>

#include <DirectXMath.h>
#include <grafkit/core/ErrorCode.h>
#include <grafkit/core/ShaderStage.h>
#include <type_traits>

namespace Grafkit::Core
{

	struct Buffer
	{
		ID3D11Buffer* buffer;
		uint32_t stride;
	};

	struct RenderTarget
	{
		mutable ID3D11RenderTargetView* renderTargetView = nullptr;
		mutable ID3D11ShaderResourceView* shaderResourceView = nullptr;
		mutable ID3D11DepthStencilView* depthStencilView = nullptr;
		mutable ID3D11Texture2D* texture = nullptr;
		mutable ID3D11Texture2D* depthTexture = nullptr;
		mutable uint32_t width = 0;
		mutable uint32_t height = 0;
	};

	struct Shader
	{
		ID3D11VertexShader* vertex = nullptr;
		ID3D11PixelShader* pixel = nullptr;
		ID3D11InputLayout* layout = nullptr;
		ShaderStage stage;
	};

	class Renderer
	{
	public:
		explicit Renderer() = default;

		void Shutdown();

		Renderer(const Renderer&) = delete;
		Renderer(Renderer&&) = delete;
		Renderer& operator=(Renderer&&) = delete;
		Renderer& operator=(const Renderer&) { return *this; }

		[[nodiscard]] ErrorCode
		CreateDeviceAndSwapChainForHwnd(HWND windowHandle, size_t width, size_t height, const RenderTarget& backBuffer) noexcept;

		template <typename TType>
		[[nodiscard]] ErrorCode CreateVertexBufferWithData(const TType* Data, const size_t Count, Buffer& Buffer) const noexcept;
		[[nodiscard]] ErrorCode CreateIndexBufferWithData(const uint32_t* Data, const size_t Count, Buffer& Buffer) const noexcept;
		template <typename TType> ErrorCode CreateConstantBufferWithData(const TType& Data, Buffer& Buffer) const noexcept;
		[[nodiscard]] ErrorCode CreateVertexShader(
			const wchar_t* FileName,
			const char* EntryPoint,
			const D3D11_INPUT_ELEMENT_DESC* InputElementDescriptorArray,
			const size_t InputElementCount,
			Shader& Shader) const noexcept;
		[[nodiscard]] ErrorCode CreatePixelShader(const wchar_t* FileName, const char* EntryPoint, Shader& Shader) const noexcept;
		[[nodiscard]] ErrorCode
		CreateRenderTarget(uint32_t Width, uint32_t Height, DXGI_FORMAT Format, RenderTarget& RenderTarget) const noexcept;
		[[nodiscard]] ErrorCode
		CreateDepthStencil(uint32_t Width, uint32_t Height, DXGI_FORMAT Format, const RenderTarget& DepthStencil) const noexcept;
		[[nodiscard]] ErrorCode CreateTextureFromFile(const char* FileName, const DXGI_FORMAT Format, RenderTarget& Texture) const noexcept;
		[[nodiscard]] ErrorCode CreateTextureFromMemory(
			const uint8_t* Data,
			uint32_t Width,
			uint32_t Height,
			uint8_t Components,
			DXGI_FORMAT Format,
			RenderTarget& Texture) const noexcept;
		[[nodiscard]] ErrorCode CreateCubeMapTexture(const char* Directory, RenderTarget& CubeMap) const noexcept;

		void DestroyBuffer(Buffer& Buffer) const noexcept;
		void DestroyShader(Shader& Shader) const noexcept;
		void DestroyRenderTarget(RenderTarget& RenderTarget) const noexcept;
		void DestroyTexture(RenderTarget& Texture) const noexcept;

		void ResizeBackBuffer(const uint32_t Width, const uint32_t Height, const RenderTarget& BackBuffer) const noexcept;
		template <typename TType> void UpdateSubResource(const Buffer& Buffer, const TType* Data, const size_t ByteSize) const noexcept;

		void ClearRenderTarget(const RenderTarget& RenderTarget, const DirectX::XMFLOAT4& Colour) const noexcept;
		void ClearDepthStencil(const RenderTarget& RenderTarget, const uint32_t ClearFlags, const float Depth, const uint8_t Stencil)
			const noexcept;

		void UnbindRenderTargets() const noexcept;

		void SetConstantBuffer(const Buffer& ConstantBuffer, const ShaderStage ShaderStage, const size_t Slot = 0) const noexcept;
		void SetViewport(
			uint32_t Width,
			uint32_t Height,
			uint32_t XOffset = 0,
			uint32_t YOffset = 0,
			float MinDepth = 0.0f,
			float MaxDepth = 1.0f) const noexcept;
		void SetShader(const Shader& shader) const noexcept;
		void SetRenderTarget(const RenderTarget& renderTarget) const noexcept;
		void SetRenderTargets(const size_t count, const RenderTarget* renderTarget) const noexcept;
		void SetTexture(const uint32_t slot, const RenderTarget& texture) const noexcept;
		void SetPrimitiveTopology(const D3D11_PRIMITIVE_TOPOLOGY PrimitiveTopology) const noexcept;
		void SetVertexBuffer(const size_t StartSlot, const Buffer& Buffer, const uint32_t Offset) const noexcept;
		void SetIndexBuffer(const size_t StartSlot, const Buffer& Buffer, const uint32_t Offset) const noexcept;

		void Draw(const size_t vertexCount, const size_t vertexLocationStart) const noexcept;
		void DrawIndexed(const size_t indexCount, const size_t indexLocationStart = 0, const size_t vertexLocationBase = 0) const noexcept;

		[[nodiscard]] ErrorCode Present(const size_t syncInterval = 0, const size_t flags = 0) const noexcept;

	private:
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_deviceContext;
		IDXGISwapChain* m_swapChain;
		ID3D11SamplerState* m_linearWrapSampler;
		ID3D11SamplerState* m_linearClampSampler;
	};

	template <typename TType>
	[[nodiscard]] ErrorCode Renderer::CreateVertexBufferWithData(const TType* Data, const size_t Count, Buffer& Buffer) const noexcept
	{
		D3D11_BUFFER_DESC bufferDesc {};
		bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
		bufferDesc.ByteWidth = sizeof(TType) * Count;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

		Buffer.stride = sizeof(TType);

		D3D11_SUBRESOURCE_DATA InitialData {};
		InitialData.pSysMem = Data;
		InitialData.SysMemPitch = 0;
		InitialData.SysMemSlicePitch = 0;

		const auto HResult = m_device->CreateBuffer(&bufferDesc, &InitialData, &Buffer.buffer);
		if (HResult != S_OK)
		{
			return ErrorCode::FAIL;
		}

		return ErrorCode::OK;
	}

	template <typename TType>
	[[nodiscard]] ErrorCode Renderer::CreateConstantBufferWithData(const TType& Data, Buffer& Buffer) const noexcept
	{
		D3D11_BUFFER_DESC bufferDesc {};
		bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
		bufferDesc.ByteWidth = (sizeof(TType) | 15) + 1;
		bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

		const auto HResult = m_device->CreateBuffer(&bufferDesc, nullptr, &Buffer.buffer);
		if (HResult != S_OK)
		{
			return ErrorCode::FAIL;
		}

		UpdateSubResource(Buffer, &Data, sizeof(TType));
		return ErrorCode::OK;
	}

	template <typename TType>
	void Renderer::UpdateSubResource(const Buffer& Buffer, const TType* Data, const size_t ByteSize) const noexcept
	{
		D3D11_MAPPED_SUBRESOURCE mappedSubResource {};
		m_deviceContext->Map(Buffer.buffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &mappedSubResource);
		memcpy(mappedSubResource.pData, Data, ByteSize);
		m_deviceContext->Unmap(Buffer.buffer, NULL);
	}

}
