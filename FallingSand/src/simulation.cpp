#include "simulation.h"

#include <iostream>

Simulation::Simulation(Grid* grid) : mt(rd()), dist(0.0f, 1.0f), grid(grid), gravity(4.0f)
{
	// TODO: make gravity changeable at runtime
	assert(grid);
}

void Simulation::update(float delta)
{
	simulate(delta);
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
		int e2 = 2 * error;
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

		if (!grid->is_air(x, y))
		{
			return { prev_x, prev_y };
		}

		prev_x = x;
		prev_y = y;
	}
}

void Simulation::simulate(float delta)
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

			if (!particle->simulate_reverse)
			{
				if (particle->has_gravity)
				{
					// TODO: use bitmask to determine whether to apply gravity
					if (grid->is_denser(particle, x, y + 1))
						particle->velocity.y += gravity * delta;

					// try to move to next position with velocity

					// TODO: replace with thread safe random!!!
					auto vx = dist(mt) < 0.5f ? ceil(particle->velocity.x) : floor(particle->velocity.x);
					auto vy = dist(mt) < 0.5f ? ceil(particle->velocity.y) : floor(particle->velocity.y);

					// std::cout << vx << ", " << vy << std::endl;
					auto rc = raycast(x, y, vx, vy);
					// move the particle to raycasted empty position
					if (x != rc.x || y != rc.y)
						grid->swap(x, y, rc.x, rc.y);
				}

				particle->life_time -= delta;
			}

			switch (particle->type)
			{
			case Particle::SAND:
				sand(particle, x, y);
				break;
			case Particle::WATER:
				water(particle, x, y);
				break;
			default:
				break;
			}
		}
	}

	// iterate from top to bottom for rising particles
	for (int y = 0; y < grid->get_height(); ++y)
	{
		auto rand = dist(mt);
		for (int xr = 0; xr < grid->get_width(); ++xr)
		{
			// randomize row iteration order
			auto x = rand < 0.5f ? xr : grid->get_width() - 1 - xr;
			auto particle = grid->get(x, y);

			if (particle->simulate_reverse)
			{
				particle->life_time -= delta;
				
				switch (particle->type)
				{
				case Particle::SMOKE:
					smoke(particle, x, y);
					break;
				case Particle::FIRE:
					fire(particle, x, y);
					break;
				default:
					break;
				}
			}
		}
	}
}

void Simulation::sand(Particle* p, int x, int y)
{
	if (grid->is_denser(p, x, y + 1))
	{
		grid->swap(x, y, x, y + 1);
	}
	else if (grid->is_denser(p, x - 1, y + 1))
	{
		grid->swap(x, y, x - 1, y + 1);
	}
	else if (grid->is_denser(p, x + 1, y + 1))
	{
		grid->swap(x, y, x + 1, y + 1);
	}
	else
	{
		p->velocity.y = 0;
	}
}

void Simulation::water(Particle* p, int x, int y)
{
	if (grid->is_denser(p, x, y + 1))
	{
		grid->swap(x, y, x, y + 1);
	}
	else if (grid->is_denser(p, x - 1, y + 1))
	{
		grid->swap(x, y, x - 1, y + 1);
	}
	else if (grid->is_denser(p, x + 1, y + 1))
	{
		grid->swap(x, y, x + 1, y + 1);
	}
	else if (grid->is_denser(p, x - 1, y))
	{
		grid->swap(x, y, x - 1, y);
	}
	else if (grid->is_denser(p, x + 1, y))
	{
		grid->swap(x, y, x + 1, y);
	}
	else
	{
		p->velocity.y = 0;
	}
}

void Simulation::smoke(Particle* p, int x, int y)
{
	if (p->life_time < 0)
	{
		// TODO: decrease alpha with lifetime
		grid->set(x, y, Particle::EMPTY);
		return;
	}
	if (grid->is_denser(p, x, y - 1))
	{
		grid->swap(x, y, x, y - 1);
	}
	else if (grid->is_denser(p, x - 1, y - 1))
	{
		grid->swap(x, y, x - 1, y - 1);
	}
	else if (grid->is_denser(p, x + 1, y - 1))
	{
		grid->swap(x, y, x + 1, y - 1);
	}
}

void Simulation::fire(Particle* p, int x, int y)
{
	if (!grid->catches_fire(x, y))
		p->life_time = std::min(p->life_time, 0.2f);

	if (p->life_time < 0)
	{
		grid->set(x, y, Particle::SMOKE);
		return;
	}

	int dx[] = { 1, 1, 0, -1, -1, -1,  0,  1 };
	int dy[] = { 0, 1, 1,  1,  0, -1, -1, -1 };

	for (int i = 0; i < 8; ++i)
	{
		int nx = x + dx[i];
		int ny = y + dy[i];
		if (p->life_time < 0.2f && grid->catches_fire(nx, ny))
			grid->set(nx, ny, Particle::FIRE);
	}
}