#include "grid.h"

#include <iostream>
#include <ostream>

Grid::Grid(unsigned int width, unsigned int height) : width(width), height(height), mt(rd()), dist(0.0f, 1.0f)
{
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

void Grid::set(int x, int y, Particle::Type particle_type)
{
	if (x >= width || y >= height)
	{
#ifdef DEBUG
		std::cerr << "SET Out of range: " << x << ", " << y << std::endl;
#endif
		return;
	}

	Particle particle;
	switch (particle_type)
	{
	case Particle::SAND:
		particle = {
			.color = Color_Util::vary_color(Color(0xFFD700)),
			.density = 100,
			.is_air = false,
			.has_gravity = true,
		};
		break;
	case Particle::WATER:
		particle = {
			.color = Color(0x0000FF),
			.density = 50,
			.is_air = false,
			.has_gravity = true,
		};
		break;
	case Particle::WOOD:
		particle = {
			.color = Color(0x362312),
			.density = 200,
			.flammability = 0.1,
			.is_air = false,
		};
		break;
	case Particle::SMOKE:
		particle = {
			.color = Color_Util::vary_color(Color(0x888888)),
			.life_time = 0.5f + 1.5f * dist(mt),
			.density = 1,
			.simulate_reverse = true,
		};
		break;
	case Particle::FIRE:
		particle = {
			.color = Color_Util::vary_color(Color(0xFF4500)),
			.life_time = 3.0f + 2.0f * dist(mt),
			.density = 2,
			.simulate_reverse = true,
		};
		break;
	default:
		break;
	}
	particle.type = particle_type;
	grid[y * width + x] = particle;
}

void Grid::swap(int x1, int y1, int x2, int y2)
{
	auto xy1 = get(x1, y1);
	auto xy2 = get(x2, y2);
	if (!xy1 || !xy2) return;
	Particle temp = *xy1;
	grid[y1 * width + x1] = *xy2;
	grid[y2 * width + x2] = temp;
}

bool Grid::is_air(int x, int y)
{
	if (!is_valid(x, y)) return false; // assume out of bounds is solid
	return get(x, y)->is_air;
}

bool Grid::is_denser(Particle* particle, int x, int y)
{
	if (!is_valid(x, y)) return false; // assume out of bounds is solid
	return get(x, y)->density < particle->density;
}

bool Grid::catches_fire(int x, int y)
{
	if (!is_valid(x, y)) return false;
	return get(x, y)->flammability > dist(mt);
}