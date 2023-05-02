#include <grafkit/render/Camera.hpp>

using namespace DirectX;

using namespace Grafkit::Core;
using namespace Grafkit::Render;

Camera::Camera(Renderer& Renderer)
	: m_InternalRenderer(Renderer)
{
}

Camera::~Camera() { m_InternalRenderer.DestroyBuffer(ConstantBuffer); }

void Camera::Initialize(const uint32_t Width, const uint32_t Height) noexcept
{

	Position = XMVectorSet(0.0f, 5.0f, -10.0f, 0.0f);
	Target = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
	Up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
	View = XMMatrixLookAtLH(Position, Target, Up);
	const auto AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
	Projection = XMMatrixPerspectiveFovLH(0.4f * 3.14f, AspectRatio, 0.1f, 1000.0f);

	OnUpdate(0.0f);

	RotationMatrix = XMMatrixRotationRollPitchYaw(Pitch, Yaw, 0);
	Target = XMVector3TransformCoord(DefaultForward, RotationMatrix);
	Target = XMVector3Normalize(Target);

	Right = XMVector3TransformCoord(DefaultRight, RotationMatrix);
	Forward = XMVector3TransformCoord(DefaultForward, RotationMatrix);
	Up = XMVector3Cross(Forward, Right);

	Position += LeftRight * Right;
	Position += BackForward * Forward;

	LeftRight = 0.0f;
	BackForward = 0.0f;

	Target = Position + Target;

	View = XMMatrixLookAtLH(Position, Target, Up);

	XMStoreFloat4(&CameraConstants.Position, Position);

	m_InternalRenderer.CreateConstantBufferWithData(CameraConstants, ConstantBuffer);
}

void Camera::OnUpdate(const float Time) noexcept
{

	RotationMatrix = XMMatrixRotationRollPitchYaw(Pitch, Yaw, 0);
	Target = XMVector3TransformCoord(DefaultForward, RotationMatrix);
	Target = XMVector3Normalize(Target);

	Right = XMVector3TransformCoord(DefaultRight, RotationMatrix);
	Forward = XMVector3TransformCoord(DefaultForward, RotationMatrix);
	Up = XMVector3Cross(Forward, Right);

	Position += LeftRight * Right;
	Position += BackForward * Forward;

	LeftRight = 0.0f;
	BackForward = 0.0f;

	Target = Position + Target;

	View = XMMatrixLookAtLH(Position, Target, Up);
}

void Camera::OnRender() noexcept
{
	XMStoreFloat4(&CameraConstants.Position, Position);
	m_InternalRenderer.SetConstantBuffer(ConstantBuffer, ShaderStage::PIXEL);
	m_InternalRenderer.UpdateSubResource(ConstantBuffer, &CameraConstants, sizeof(SCameraConstants));
}

XMMATRIX Camera::GetViewMatrix() const noexcept { return View; }

XMMATRIX Camera::GetProjectionMatrix() const noexcept { return Projection; }
