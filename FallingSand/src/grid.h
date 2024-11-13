#pragma once

#include <cstdint>
#include <random>
#include <DirectXMath.h>

#include "color.h"
#include <tsl/robin_map.h>

struct Particle
{
    enum Type : uint16_t
    {
        SAND,
        WATER,
        WOOD,
        SMOKE,
        FIRE,
        EMPTY,
        END,
    };
    // TOTAL = 32
    XMFLOAT2 velocity = { 0, 0 }; // 8
    Color color = 0x000000; // 4
    float life_time = 0; // 4
    float density = 0; // 4
    float flammability = 0; // 4
    Type type = EMPTY; // 2
    uint16_t is_air = true; // 2
    uint16_t has_gravity = false; // 2
    uint16_t simulate_reverse = false; // 2
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
	bool is_air(int x, int y);
	bool is_denser(Particle* particle, int x, int y);
	bool catches_fire(int x, int y);
	bool is_valid(int x, int y) const { return x >= 0 && x < (int) width && y>= 0 && y < (int) height; }
};
