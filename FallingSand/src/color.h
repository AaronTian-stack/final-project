#pragma once

#include <cstdint>
#include <DirectXMath.h>

using namespace DirectX;

struct Color
{
	uint32_t hexValue; // leading two hex digits are not used (no alpha)
	Color() : hexValue(0) {}
	Color(uint32_t hexValue) : hexValue(hexValue) {}
	Color(uint8_t r, uint8_t g, uint8_t b) : hexValue((r << 16) | (g << 8) | b) {}
	uint8_t r() const { return (hexValue >> 16) & 0xFF; }
	uint8_t g() const { return (hexValue >> 8) & 0xFF; }
	uint8_t b() const { return hexValue & 0xFF; }
	uint32_t hex() const { return hexValue; }

	XMFLOAT3 to_hsl() const;
	void from_hsl(float h, float s, float l);
};

struct Color_Util
{
    template<typename T>
	static T generate(T min, T max);
	static Color vary_color(Color& color);
};