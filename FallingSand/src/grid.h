#pragma once

#include <cstdint>
#include <random>
#include <DirectXMath.h>

#include "color.h"
#include <tsl/robin_map.h>

struct Particle
{
	enum Type
	{
		SAND,
		WATER,
		STONE,
		WOOD,
		SMOKE,
		FIRE,
		SALT,
		ACID,
		EMPTY,
	};
	enum Matter
	{
		AIR,
		LIQUID,
		SOLID,
	};
	Type type = EMPTY;
	Matter matter = AIR;
	XMFLOAT2 velocity = { 0, 0 };
	Color color = 0x000000;
	float life_time = 0;
	float density = 0;
	float flammability = 0;
	float dissolvability = 0;
	float corrodibility = 0;
	bool has_gravity = false;
	bool simulate_reverse = false;
	bool burning = false;
	bool dying = false;
};

class Grid
{
	std::random_device rd;
	std::mt19937 mt;
	std::uniform_real_distribution<float> dist;
	Particle* grid; // save overhead of size, capacity from vector
	unsigned int width;
	unsigned int height;
public:
	Grid(unsigned int width, unsigned int height);
	~Grid();
	Particle* get(int x, int y) const;
	void set(int x, int y, Particle::Type particle);
	void swap(int x1, int y1, int x2, int y2);
	unsigned int get_width() const { return width; }
	unsigned int get_height() const { return height; }

	bool is_valid(int x, int y) const { return x >= 0 && x < (int)width && y >= 0 && y < (int)height; }
	bool is_air(int x, int y);
	bool is_liquid(int x, int y);
	bool is_solid(int x, int y);
	bool is_burning(int x, int y);
	bool is_denser(Particle* particle, int x, int y);
	float corrodibility(int x, int y);
};
