#pragma once

#include <cstdint>
#include <DirectXMath.h>

using namespace DirectX;

struct Color
{
	uint32_t hex_value; // leading two hex digits are not used (no alpha)
	Color() : hex_value(0) {}
	Color(uint32_t hexValue) : hex_value(hexValue) {}
	Color(const uint8_t r, const uint8_t g, const uint8_t b) : hex_value((r << 16) | (g << 8) | b) {}
	[[nodiscard]] uint8_t r() const { return (hex_value >> 16) & 0xFF; }
	[[nodiscard]] uint8_t g() const { return (hex_value >> 8) & 0xFF; }
	[[nodiscard]] uint8_t b() const { return hex_value & 0xFF; }
	[[nodiscard]] uint32_t hex() const { return hex_value; }

	[[nodiscard]] XMFLOAT3 to_hsl() const;
	void from_hsl(float h, float s, float l);
	[[nodiscard]] Color mix(const Color& other, float amount) const
	{
		auto a = 1.0f - amount;
		return Color(
			static_cast<uint8_t>(r() * a + other.r() * amount),
			static_cast<uint8_t>(g() * a + other.g() * amount),
			static_cast<uint8_t>(b() * a + other.b() * amount)
		);
	}
};

struct Color_Util
{
    template<typename T>
	static T generate(T min, T max);
	static Color vary_color(const Color& color);
};