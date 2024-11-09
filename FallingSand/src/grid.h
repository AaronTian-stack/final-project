#pragma once

#include <cstdint>
#include <DirectXMath.h>

#include "color.h"

// https://www.intel.com/content/www/us/en/docs/programmable/683176/18-1/aligning-a-struct-with-or-witout-padding.html#:~:text=A%20proper%20struct%20alignment%20means,increases%20with%20the%20increasing%20alignment
struct Particle
{
	enum ID : uint16_t
	{
		EMPTY = 0,
		SAND = 1,
		WATER = 2,
		END,
	};
	ID id = EMPTY; // 2
	uint16_t updated = 0; // 2
	float life = 0; // 4
	XMFLOAT2 velocity = { 0, 0 }; // 8
	Color color = 0x000000; // 4
};

class Grid
{
	const Particle OUT_OF_BOUNDS = { Particle::END, 0, 0, {0, 0}, 0x000000 };
	Particle* grid; // save overhead of size, capacity from vector
	unsigned int width;
	unsigned int height;
public:
	Grid(unsigned int width, unsigned int height);
	~Grid();
	Particle* get(int x, int y) const;
	void set(int x, int y, const Particle& particle);
	void swap(int x1, int y1, int x2, int y2);
	unsigned int get_width() const { return width; }
	unsigned int get_height() const { return height; }
	bool is_empty(int x, int y);
	bool is_valid(int x, int y) const { return x >= 0 && x < width && y>= 0 && y < height; }
};
