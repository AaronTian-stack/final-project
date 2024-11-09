#pragma once
#include <random>

#include "grid.h"

class Simulation
{
	// TODO: for multithreading will need one of these for each thread
	std::random_device rd;
	std::mt19937 mt;
	std::uniform_real_distribution<float> dist;
	Grid* grid;
	float gravity;
public:
	Simulation(Grid* grid);
	void update();

	// returns closest position of particle in velocity (vx, vy) from (x, y)
	XMINT2 raycast(int x, int y, int vx, int vy);

	// Only particles with gravity can be simulated bottom to top. Would not work with smoke for example
	void simulate_bottom_to_top();
	// TODO: insert logic to update individual particles
	void sand(Particle& particle, int x, int y);
};
