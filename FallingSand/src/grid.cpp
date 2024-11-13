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
			.corrodibility = 0.05,
			.matter = Particle::SOLID,
			.has_gravity = true,
		};
		break;
	case Particle::WATER:
		particle = {
			.color = Color(0x0000FF),
			.density = 50,
			.matter = Particle::LIQUID,
			.has_gravity = true,
		};
		break;
	case Particle::STONE:
		particle = {
			.color = Color(0x696969),
			.density = 500,
			.matter = Particle::SOLID,
		};
		break;
	case Particle::WOOD:
		particle = {
			.color = Color(0x362312),
			.density = 200,
			.flammability = 0.025,
			.corrodibility = 0.1,
			.matter = Particle::SOLID,
		};
		break;
	case Particle::SMOKE:
		particle = {
			.color = Color_Util::vary_color(Color(0x888888)),
			.life_time = 0.1f + 2.0f * dist(mt),
			.density = 1,
			.matter = Particle::AIR,
			.simulate_reverse = true,
			.dying = true,
		};
		break;
	case Particle::FIRE:
		particle = {
			.color = Color_Util::vary_color(Color(0xFF4500)),
			.life_time = 0.1f + 0.1f * dist(mt),
			.density = 2,
			.dissolvability = 0.5,
			.matter = Particle::AIR,
			.simulate_reverse = true,
			.burning = true,
			.dying = true,
		};
		break;
	case Particle::SALT:
		particle = {
			.color = Color_Util::vary_color(Color(0xDDDDDD)),
			.life_time = 0.5f + 0.5f * dist(mt),
			.density = 90,
			.dissolvability = 0.05,
			.corrodibility = 0.25,
			.matter = Particle::SOLID,
			.has_gravity = true,
		};
		break;
	case Particle::ACID:
		particle = {
			.color = Color(0x8FFE09),
			.life_time = 5.0f + 5.0f * dist(mt),
			.density = 60,
			.dissolvability = 0.005,
			.matter = Particle::LIQUID,
			.has_gravity = true,
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
	if (!is_valid(x, y)) return false;
	return get(x, y)->matter == Particle::AIR;
}

bool Grid::is_liquid(int x, int y)
{
	if (!is_valid(x, y)) return false;
	return get(x, y)->matter == Particle::LIQUID;
}

bool Grid::is_solid(int x, int y)
{
	if (!is_valid(x, y)) return false;
	return get(x, y)->matter == Particle::SOLID;
}

bool Grid::is_burning(int x, int y)
{
	if (!is_valid(x, y)) return false;
	return get(x, y)->burning;
}

bool Grid::is_denser(Particle* particle, int x, int y)
{
	if (!is_valid(x, y)) return false; // assume out of bounds infinitely dense
	return get(x, y)->density < particle->density;
}

float Grid::corrodibility(int x, int y)
{
	if (!is_valid(x, y)) return 0;
	return get(x, y)->corrodibility;
}