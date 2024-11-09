#include "color.h"

#include <algorithm>
#include <random>

XMFLOAT3 Color::to_hsl() const
{
	float r = static_cast<float>(this->r()) / 255.0f;
	float g = static_cast<float>(this->g()) / 255.0f;
	float b = static_cast<float>(this->b()) / 255.0f;

	float max = std::max(r, std::max(g, b));
	float min = std::min(r, std::min(g, b));

	float h, s;

	float l = (max + min) * 0.5f;

	if (max == min)
	{
		h = s = 0.0f;
	}
	else
	{
		float d = max - min;
		s = l > 0.5f ? d / (2.0f - max - min) : d / (max + min);

		if (max == r)
		{
			h = (g - b) / d + (g < b ? 6.0f : 0.0f);
		}
		else if (max == g)
		{
			h = (b - r) / d + 2.0f;
		}
		else
		{
			h = (r - g) / d + 4.0f;
		}

		h /= 6.0f;
	}

	return { h, s, l };
}

void Color::from_hsl(float h, float s, float l)
{
	float r, g, b;

	if (s == 0.0f)
	{
		r = g = b = l; // achromatic
	}
	else
	{
		auto hue2rgb = [](float p, float q, float t)
			{
				if (t < 0.0f) t += 1.0f;
				if (t > 1.0f) t -= 1.0f;
				if (t < 1.0f / 6.0f) return p + (q - p) * 6.0f * t;
				if (t < 1.0f / 2.0f) return q;
				if (t < 2.0f / 3.0f) return p + (q - p) * (2.0f / 3.0f - t) * 6.0f;
				return p;
			};

		float q = l < 0.5f ? l * (1.0f + s) : l + s - l * s;
		float p = 2.0f * l - q;
		r = hue2rgb(p, q, h + 1.0f / 3.0f);
		g = hue2rgb(p, q, h);
		b = hue2rgb(p, q, h - 1.0f / 3.0f);
	}

	this->hexValue = (static_cast<uint32_t>(r * 255.0f) << 16) | (static_cast<uint32_t>(g * 255.0f) << 8) | static_cast<uint32_t>(b * 255.0f);
}

template<typename T>
T Color_Util::generate(T min, T max)
{
	static std::random_device rd;
	static std::mt19937 gen(rd());
	if constexpr (std::is_integral_v<T>)
	{
		std::uniform_int_distribution<T> dist(min, max);
		return dist(gen);
	}
	else if constexpr (std::is_floating_point_v<T>)
	{
		std::uniform_real_distribution<T> dist(min, max);
		return dist(gen);
	}
	assert(false);
	return {};
}

Color Color_Util::vary_color(Color& color)
{
	auto hsl = color.to_hsl();

	auto g1 = generate(-0.2f, 0.f);
	auto g2 = generate(-0.1f, 0.1f);

	auto saturation = hsl.y + g1;
	saturation = std::clamp(saturation, 0.f, 1.f);
	auto lightness = hsl.z + g2;
	lightness = std::clamp(lightness, 0.f, 1.f);

	Color c;
	c.from_hsl(hsl.x, saturation, lightness);
	return c;
}
