#pragma once

#include <cstdint>
#include <random>
#include <DirectXMath.h>

#include "color.h"
#include <tsl/robin_map.h>

// Threadsafe random generator
inline float thread_rand()
{
	static thread_local std::mt19937 generator(std::random_device{}());
	std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
	return distribution(generator);
}

struct Particle
{
	enum Type : uint16_t
	{
		SAND = 1 << 0,
		WATER = 1 << 1,
		STONE = 1 << 2,
		WOOD = 1 << 3,
		SMOKE = 1 << 4,
		FIRE = 1 << 5,
		SALT = 1 << 6,
		ACID = 1 << 7,
		GASOLINE = 1 << 8,
		EMPTY = 1 << 9,
	};
	// TOTAL = 36 bytes
	XMFLOAT2 velocity = { 0, 0 }; // 8
	Color color = 0x000000; // 4
	float life_time = 0; // 4
	float density = 0; // 4
	float flammability = 0; // 4
	float dissolvability = 0; // 4
	float corrodibility = 0; // 4
	Type type = EMPTY; // 2
	// TODO: pack these states into leftover bits of Type
	uint8_t dying = 0; // 1
	uint8_t burning = 0; // 1
};

struct ParticleUtils
{
	inline static tsl::robin_map<Particle::Type, Color> colors =
	{
		{ Particle::EMPTY, Color(0x000000) },
		{ Particle::SAND, Color(0xFFD700) },
		{ Particle::WATER, Color(0x0000FF) },
		{ Particle::STONE, Color(0x696969) },
		{ Particle::WOOD, Color(0x362312) },
		{ Particle::SMOKE, Color(0x888888) },
		{ Particle::FIRE, Color(0xFF4500) },
		{ Particle::SALT, Color(0xDDDDDD) },
		{ Particle::ACID, Color(0x8FFE09) },
		{ Particle::GASOLINE, Color(0xFFA500) },
	};

	static bool is_solid(Particle::Type type)
	{
		return type & (Particle::SAND | Particle::STONE | Particle::WOOD | Particle::SALT);
	}

	static bool is_liquid(Particle::Type type)
	{
		return type & (Particle::WATER | Particle::ACID | Particle::GASOLINE);
	}

	static bool is_air(Particle::Type type)
	{
		return type & (Particle::SMOKE | Particle::EMPTY);
	}

	static bool affected_by_gravity(Particle::Type type)
	{
		return type & (Particle::SAND | Particle::WATER | Particle::SALT | Particle::ACID | Particle::GASOLINE);
	}

	static bool reversed_simulation(Particle::Type type)
	{
		return type & (Particle::SMOKE | Particle::FIRE);
	}

	static bool use_solid_brush(Particle::Type type)
	{
		return type & (Particle::EMPTY | Particle::STONE | Particle::WOOD);
	}
};

class Grid
{
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

	bool is_valid(int x, int y) const;
	bool is_air(int x, int y);
	bool is_liquid(int x, int y);
	bool is_solid(int x, int y);
	bool is_burning(int x, int y);
	bool is_denser(Particle* particle, int x, int y);
};

inline bool Grid::is_valid(int x, int y) const
{
	return x >= 0 && x < width && y >= 0 && y < height;
}
