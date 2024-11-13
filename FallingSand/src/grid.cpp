#include "grid.h"

#include <iostream>
#include <ostream>

Grid::Grid(unsigned width, unsigned height)
{
	this->width = width;
	this->height = height;
	this->grid = new Particle[static_cast<size_t>(width * height)];
}

Grid::~Grid()
{
	delete[] grid;
}

Particle* Grid::get(int x, int y) const
{
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
#ifdef DEBUG
		std::cerr << "GET Out of range: " << x << ", " << y << std::endl;
#endif
		return nullptr;

	}
	return &grid[y * width + x];
}

void Grid::set(int x, int y, const Particle& particle)
{
	if (x >= width || y >= height)
	{
#ifdef DEBUG
		std::cerr << "SET Out of range: " << x << ", " << y << std::endl;
#endif
		return;
	}
	grid[y * width + x] = particle;
}

void Grid::swap(int x1, int y1, int x2, int y2)
{
	auto xy1 = get(x1, y1);
	auto xy2 = get(x2, y2);
	if (!xy1 || !xy2) return;
	Particle temp = *xy1;
	set(x1, y1, *xy2);
	set(x2, y2, temp);
}

bool Grid::is_empty(int x, int y)
{
	if (!is_valid(x, y)) return false; // assume out of bounds is solid
	return get(x, y)->id == Particle::EMPTY;
}
