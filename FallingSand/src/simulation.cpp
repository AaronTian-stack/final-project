#include "simulation.h"

Simulation::Simulation(Grid* grid) : mt(rd()), dist(0.0f, 1.0f), grid(grid), gravity(0.2f)
{
	assert(grid);
}

void Simulation::update()
{
	simulate_bottom_to_top();
}

XMINT2 Simulation::raycast(int x, int y, int vx, int vy)
{
	// don't really care about a precise position of particles so just use bresenham
	int x1 = x + vx;
	int y1 = y + vy;
	
	int dx = abs(x1 - x);
	int sx = x < x1 ? 1 : -1;
	int dy = -abs(y1 - y);
	int sy = y < y1 ? 1 : -1;
	int error = dx + dy;

	int prev_x = x;
	int prev_y = y;

	while (true)
	{
		if (x == x1 && y == y1) return { x1, y1 };
;		int e2 = 2 * error;
		if (e2 >= dy)
		{
			error = error + dy;
			x = x + sx;
		}
		if (e2 <= dx)
		{
			error = error + dx;
			y = y + sy;
		}

		auto get = grid->get(x, y);
		if (!get || grid->get(x, y)->id != Particle::EMPTY)
		{
			return { prev_x, prev_y };
		}

		prev_x = x;
		prev_y = y;
	}
}

void Simulation::simulate_bottom_to_top()
{
	// iterate from bottom to top
	for (int y = grid->get_height() - 1; y >= 0; --y)
	{
		auto rand = dist(mt);
		for (int xr = 0; xr < grid->get_width(); ++xr)
		{
			// randomize row iteration order
			auto x = rand < 0.5f ? xr : grid->get_width() - 1 - xr;
			auto particle = grid->get(x, y);

			// TODO: apply gravity

			// TODO: replace with switch statement
			if (particle->id == Particle::SAND)
			{
				// TODO: use bitmask to determine whether to apply gravity
				auto below = grid->get(x, y + 1);
				if (below && below->id == Particle::EMPTY)
					particle->velocity.y += gravity;

				sand(*particle, x, y);
			}
		}
	}
}

void Simulation::sand(Particle& particle, int x, int y)
{
	// try to move to next position with velocity

	// TODO: replace with thread safe random!!!
	auto vx = rand() < 0.5 ? ceil(particle.velocity.x) : floor(particle.velocity.x);
	auto vy = rand() < 0.5 ? ceil(particle.velocity.y) : floor(particle.velocity.y);

	auto rc = raycast(x, y, vx, vy);
	// move the particle to raycasted empty position
	if (x != rc.x || y != rc.y)
		grid->swap(x, y, rc.x, rc.y);

	// then apply sifting logic
	if (y < grid->get_height() - 1)
	{
		auto below = grid->get(x, y + 1);
		bool moved = false;
		if (below->id == Particle::EMPTY)
		{
			grid->swap(x, y, x, y + 1);
			moved = true;
		}
		else if (x > 0 && grid->get(x - 1, y + 1)->id == Particle::EMPTY)
		{
			grid->swap(x, y, x - 1, y + 1);
			moved = true;
		}
		else if (x < grid->get_width() - 1 && grid->get(x + 1, y + 1)->id == Particle::EMPTY)
		{
			grid->swap(x, y, x + 1, y + 1);
			moved = true;
		}
		if (!moved) particle.velocity.y = 0;
	}
}
