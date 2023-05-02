#include <grafkit/render/Model.hpp>

#define NOMINMAX

using namespace Grafkit::Core;
using namespace Grafkit::Render;

Model::Model(Renderer& Renderer, Camera& Camera)
	: m_InternalRenderer(Renderer)
	, m_InternalCamera(Camera)
{
}

Model::~Model()
{
	for (auto& Mesh : m_Meshes)
	{
		m_InternalRenderer.DestroyBuffer(Mesh.VertexBuffer);
		m_InternalRenderer.DestroyBuffer(Mesh.IndexBuffer);
	}
	m_InternalRenderer.DestroyBuffer(m_TransformConstantBuffer);
}

ErrorCode Model::Initialize(const char* Path, const uint32_t Width, const uint32_t Height) { return ErrorCode::OK; }

void Model::OnUpdate(const float Time) noexcept
{
	m_PerFrame.World = DirectX::XMMatrixTranspose(DirectX::XMMatrixRotationRollPitchYaw(m_Rotation.x, m_Rotation.y, m_Rotation.z));
	m_PerFrame.View = DirectX::XMMatrixTranspose(m_InternalCamera.GetViewMatrix());
	m_PerFrame.Projection = DirectX::XMMatrixTranspose(m_InternalCamera.GetProjectionMatrix());
}

void Model::OnRender(const RenderTarget* RenderTargets, const size_t Count) noexcept
{
	m_Material.OnRender();
	m_InternalCamera.OnRender();

	m_InternalRenderer.SetConstantBuffer(m_TransformConstantBuffer, ShaderStage::VERTEX);
	m_InternalRenderer.UpdateSubResource(m_TransformConstantBuffer, &m_PerFrame, sizeof(PerFrame));
	for (auto& Mesh : m_Meshes)
	{
		m_InternalRenderer.SetVertexBuffer(0, Mesh.VertexBuffer, 0);
		m_InternalRenderer.SetIndexBuffer(0, Mesh.IndexBuffer, 0);
		m_InternalRenderer.SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_InternalRenderer.DrawIndexed(Mesh.IndexCount);
	}
}
