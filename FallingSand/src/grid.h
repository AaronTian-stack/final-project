#pragma once

#include <cstdint>

struct Velocity
{
	float x;
	float y;
};

struct Color
{
	uint32_t hexValue; // leading two hex digits are not used (no alpha)
	Color(uint32_t hexValue) : hexValue(hexValue) {}
	Color(uint8_t r, uint8_t g, uint8_t b) : hexValue((r << 16) | (g << 8) | b) {}
	uint8_t r() const { return (hexValue >> 16) & 0xFF; }
	uint8_t g() const { return (hexValue >> 8) & 0xFF; }
	uint8_t b() const { return hexValue & 0xFF; }
	uint32_t hex() const { return hexValue; }
};

enum ID : uint16_t
{
	EMPTY = 0,
	SAND = 1,
	WATER = 2,
};

// https://www.intel.com/content/www/us/en/docs/programmable/683176/18-1/aligning-a-struct-with-or-witout-padding.html#:~:text=A%20proper%20struct%20alignment%20means,increases%20with%20the%20increasing%20alignment
struct Particle
{
	ID id = EMPTY;
	uint8_t updated = 0;
	float life = 0;
	Velocity velocity = {0, 0};
	Color color = 0x000000;
};

class Grid
{
	Particle* grid; // save overhead of size, capacity from vector
	unsigned int width;
	unsigned int height;
public:
	Grid(unsigned int width, unsigned int height);
	~Grid();
	Particle& get(unsigned int x, unsigned int y) const;
	void set(unsigned int x, unsigned int y, const Particle& particle);
	void swap(unsigned int x1, unsigned int y1, unsigned int x2, unsigned int y2);
	unsigned int get_width() const { return width; }
	unsigned int get_height() const { return height; }
	bool isEmpty(unsigned int x, unsigned int y) { return get(x, y).id == EMPTY; }
	bool isValid(unsigned int x, unsigned int y) const { return x < width && y < height; }
};
