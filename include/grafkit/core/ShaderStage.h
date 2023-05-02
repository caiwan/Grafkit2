#pragma once

#include <cstdint>
#include <type_traits>

enum class ShaderStage : uint8_t
{
	NONE = 0x00,
	VERTEX = 0x01,
	PIXEL = 0x02,
	COMPUTE = 0x04
};

inline auto operator|(const ShaderStage Lhs, const ShaderStage Rhs) -> ShaderStage
{
	using TType = std::underlying_type_t<ShaderStage>;
	return static_cast<ShaderStage>(static_cast<TType>(Lhs) | static_cast<TType>(Rhs));
}

inline auto operator|=(ShaderStage& Lhs, const ShaderStage Rhs) -> ShaderStage&
{
	Lhs = Lhs | Rhs;
	return Lhs;
}

inline auto operator&(const ShaderStage Lhs, const ShaderStage Rhs) -> ShaderStage
{
	using TType = std::underlying_type_t<ShaderStage>;
	return static_cast<ShaderStage>(static_cast<TType>(Lhs) & static_cast<TType>(Rhs));
}

inline auto operator&=(ShaderStage& Lhs, const ShaderStage Rhs) -> ShaderStage&
{
	Lhs = Lhs & Rhs;
	return Lhs;
}
