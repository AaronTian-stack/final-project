#pragma once

#include <cstdint>
#include <DirectXMath.h>

#include "color.h"
#include "grid.h"

enum ParticleType
{
	SAND,
	WATER,
	WOOD,
	SMOKE,
	FIRE,
	EMPTY,
};

class Particle
{
public:
	Particle();
	XMFLOAT2 velocity = { 0, 0 };
	Color color = 0x000000;
	float life_time = 0;
	float density = 0;
	float flammability = 0;
	bool is_air = true;
	bool has_gravity = false;
	bool simulate_reverse = false;
	void simulate_step(Grid* grid, int x, int y);
};

class Empty : public Particle
{
};

class Sand : public Particle
{
public:
	Sand();
	virtual void simulate_step(Grid* grid, int x, int y);
};

class Water : public Particle
{
public:
	Water();
	virtual void simulate_step(Grid* grid, int x, int y);
};

class Wood : public Particle
{
public:
	Wood();
};

class Smoke : public Particle
{
public:
	Smoke();
	virtual void simulate_step(Grid* grid, int x, int y);
};

class Fire : public Particle
{
public:
	Fire();
	virtual void simulate_step(Grid* grid, int x, int y);
};