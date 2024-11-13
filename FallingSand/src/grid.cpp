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
	// TODO: abstract this out better
	Particle particle;
	Color color;
	// TODO: differentiate life times for different decays: burn, dissolve, corrode
	float life_time;
	switch (particle_type)
	{
	case Particle::SAND:
		color = Color(0xFFD700);
		particle = { particle_type, Particle::SOLID, {0, 0}, Color_Util::vary_color(color), 2, 100, 0, 0, 0.05, true, false, false };
		break;
	case Particle::WATER:
		color = Color(0x0000FF);
		particle = { particle_type, Particle::LIQUID, {0, 0}, color, 0, 50, 0, 0, 0, true, false, false };
		break;
	case Particle::STONE:
		color = Color(0x696969);
		particle = { particle_type, Particle::SOLID, {0, 0}, color, 0, 500, 0, 0, 0, false, false, false };
		break;
	case Particle::WOOD:
		color = Color(0x362312);
		particle = { particle_type, Particle::SOLID, {0, 0}, color, 0, 200, 0.025, 0, 0.1, false, false, false };
		break;
	case Particle::SMOKE:
		color = Color(0x888888);
		life_time = 0.1 + 2.0 * dist(mt);
		particle = { particle_type, Particle::AIR, {0, 0}, Color_Util::vary_color(color), life_time, 1, 0, 0, 0, false, true, false, true };
		break;
	case Particle::FIRE:
		color = Color(0xFF4500);
		life_time = 0.1 + 0.1 * dist(mt);
		particle = { particle_type, Particle::AIR, { 0, 0 }, Color_Util::vary_color(color), life_time, 2, 0, 0.5, 0, false, true, true, true };
		break;
	case Particle::SALT:
		color = Color(0xDDDDDD);
		life_time = 0.5 + 0.5 * dist(mt);
		particle = { particle_type, Particle::SOLID, { 0, 0 }, Color_Util::vary_color(color), life_time, 90, 0, 0.05, 0.25, true, false, false };
		break;
	case Particle::ACID:
		color = Color(0x8FFE09);
		life_time = 5 + 5 * dist(mt);
		particle = { particle_type, Particle::LIQUID, { 0, 0 }, color, life_time, 60, 0, 0.005, 0, true, false, false };
		break;
	default:
		particle = { particle_type };
		break;
	}
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