#pragma once
#include <DirectXMath.h>
#include <grafkit/core/Renderer.h>
#include <grafkit/render/Camera.hpp>
#include <grafkit/render/Material.hpp>
#include <vector>

namespace Grafkit::Render
{

	class Model
	{

		struct Vertex
		{
			DirectX::XMFLOAT3 Position;
			DirectX::XMFLOAT3 Normal;
			DirectX::XMFLOAT2 TexCoord;
			DirectX::XMFLOAT3 Tangent;
			DirectX::XMFLOAT3 Bitangent;
		};

		struct Mesh
		{
			Core::Buffer VertexBuffer;
			Core::Buffer IndexBuffer;
			size_t IndexCount;
		};

		struct PerFrame
		{
			DirectX::XMMATRIX World;
			DirectX::XMMATRIX View;
			DirectX::XMMATRIX Projection;
		};

	public:
		explicit Model(Core::Renderer& Renderer, Camera& Camera);
		~Model();

		ErrorCode Initialize(const char* Path, const uint32_t Width, const uint32_t Height);
		void OnUpdate(const float Time) noexcept;
		void OnRender(const Core::RenderTarget* RenderTargets, const size_t Count) noexcept;

	private:
		Core::Renderer& m_InternalRenderer;
		Camera& m_InternalCamera;
		Material m_Material { m_InternalRenderer };

		DirectX::XMFLOAT3 m_Rotation;

		std::string m_FilePath;

		std::vector<Mesh> m_Meshes;

		Core::Buffer m_TransformConstantBuffer {};
		PerFrame m_PerFrame {};
	};
}
