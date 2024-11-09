#include "grid.h"

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

Particle& Grid::get(unsigned int x, unsigned int y) const
{
	return grid[y * width + x];
}

void Grid::set(unsigned int x, unsigned int y, const Particle& particle)
{
#ifdef DEBUG
	if (x >= width || y >= height)
	{
		std::cerr << "Out of range: " << x << ", " << y << std::endl;
		return;
	}
#endif
	grid[y * width + x] = particle;
}

void Grid::swap(unsigned x1, unsigned y1, unsigned x2, unsigned y2)
{
	Particle temp = grid[y1 * width + x1];
	set(x1, y1, get(x2, y2));
	set(x2, y2, temp);
}
