#pragma once
#include "grid.h"
#include <BS_thread_pool.hpp>

class Simulation
{
	Grid* grid;
	float gravity;
public:
	Simulation(Grid* grid);

	// returns closest position of particle in velocity (vx, vy) from (x, y)
	XMINT2 raycast(int x, int y, int vx, int vy);

	// Only particles with gravity can be simulated bottom to top. Would not work with smoke for example
	void update(float delta, BS::thread_pool& pool);
	void solid(Particle* particle, int x, int y);
	void liquid(Particle* particle, int x, int y);
	void air(Particle* particle, int x, int y);

	bool burns(Particle* particle, int x, int y);
	bool dissolves(Particle* particle, int x, int y);
	bool extinguishes(Particle* particle, int x, int y);

	void sand(Particle* particle, int x, int y);
	void water(Particle* particle, int x, int y);
	void smoke(Particle* particle, int x, int y);
	void wood(Particle* particle, int x, int y);
	void fire(Particle* particle, int x, int y);
	void salt(Particle* particle, int x, int y);
	void acid(Particle* particle, int x, int y);
	void gasoline(Particle* p, int x, int y);
	void vine(Particle* p, int x, int y);
};
