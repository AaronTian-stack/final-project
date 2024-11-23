#include "simulation.h"

#include <iostream>
#include <Tracy.hpp>

Simulation::Simulation(Grid* grid) : grid(grid), gravity(4.0f)
{
	// TODO: make gravity changeable at runtime
	assert(grid);
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
			return { prev_x, prev_y };

		prev_x = x;
		prev_y = y;
	}
}

void Simulation::update(float delta, BS::thread_pool& pool)
{
	ZoneScoped;
	// pick directions for each row
	std::vector<bool> directions(grid->get_height());
	for (int i = 0; i < grid->get_height(); i++)
	{
		directions[i] = thread_rand() < 0.5f;
	}

	const int num_columns = pool.get_thread_count();
	const int pixels_per_group = ceil(grid->get_width() / num_columns);

	auto iterate_bottom_to_top = [this, delta, directions](int start, int end)
		{
			if (start >= static_cast<int>(grid->get_width())) return;
			auto xr = std::min(end, static_cast<int>(grid->get_width()));
			for (int y = grid->get_height() - 1; y >= 0; --y)
			{
				for (int xi = start; xi < xr; xi++)
				{
					int x = directions[y] ? xi : xr - xi + start - 1;
					auto particle = grid->get(x, y);

					if (!ParticleUtils::reversed_simulation(particle->type))
					{
						if (ParticleUtils::affected_by_gravity(particle->type))
						{
							if (grid->is_denser(particle, x, y + 1))
								particle->velocity.y += gravity * delta;

							int vx = thread_rand() < 0.5f ? ceil(particle->velocity.x) : floor(particle->velocity.x);
							int vy = thread_rand() < 0.5f ? ceil(particle->velocity.y) : floor(particle->velocity.y);

							auto rc = raycast(x, y, vx, vy);
							if (x != rc.x || y != rc.y)
								grid->swap(x, y, rc.x, rc.y);
						}

						particle->life_time -= delta;
						if (particle->dying && particle->life_time < 0)
							grid->set(x, y, Particle::EMPTY);
					}

					switch (particle->type)
					{
					case Particle::SAND:
						sand(particle, x, y);
						break;
					case Particle::WATER:
						water(particle, x, y);
						break;
					case Particle::WOOD:
						wood(particle, x, y);
						break;
					case Particle::SALT:
						salt(particle, x, y);
						break;
					case Particle::ACID:
						acid(particle, x, y);
						break;
					case Particle::GASOLINE:
						gasoline(particle, x, y);
						break;
					case Particle::VINE:
						vine(particle, x, y);
						break;
					default:
						break;
					}
				}
			}
		};

	auto iterate_top_to_bottom = [this, delta, directions](int start, int end)
		{
			if (start >= static_cast<int>(grid->get_width())) return;
			auto xr = std::min(end, static_cast<int>(grid->get_width()));
			for (int y = 0; y < grid->get_height(); ++y)
			{
				for (int xi = start; xi < xr; xi++)
				{
					int x = directions[y] ? xi : xr - xi + start - 1;
					auto particle = grid->get(x, y);

					if (ParticleUtils::reversed_simulation(particle->type))
					{
						particle->life_time -= delta;
						if (particle->dying && particle->life_time < 0)
							grid->set(x, y, Particle::EMPTY);

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
		};

	BS::multi_future<void> futures;

	assert(num_columns % 2 == 0);
	int random_offset = thread_rand() * pixels_per_group * 0.5;

	for (int i = 0; i < num_columns; i += 2)
	{
		int start = i * pixels_per_group + random_offset * (i > 0);
		int end = (i + 1) * pixels_per_group + random_offset;
		//debug[i] = end;
		futures.push_back(pool.submit_task([=]
			{
				iterate_bottom_to_top(start, end);
				iterate_top_to_bottom(start, end);
			}
		));
	}
	futures.wait();

	for (int i = 1; i < num_columns; i += 2)
	{
		int start = i * pixels_per_group + random_offset;
		int end = (i + 1) * pixels_per_group + random_offset;
		//debug[i] = end;
		futures.push_back(pool.submit_task([=]
			{
				iterate_bottom_to_top(start, end);
				iterate_top_to_bottom(start, end);
			}
		));
	}
	futures.wait();

	//iterate_bottom_to_top(0, grid->get_width());
	//iterate_top_to_bottom(0, grid->get_width());

	//return debug;
}

void Simulation::solid(Particle* p, int x, int y)
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

void Simulation::liquid(Particle* p, int x, int y)
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

void Simulation::air(Particle* p, int x, int y)
{
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
	else if (grid->is_denser(p, x - 1, y))
	{
		grid->swap(x, y, x - 1, y);
	}
	else if (grid->is_denser(p, x + 1, y))
	{
		grid->swap(x, y, x + 1, y);
	}
}

bool Simulation::burns(Particle* p, int x, int y)
{
	float burnProbability = 0;
    std::array dx = { 1, 1, 0, -1, -1, -1,  0,  1 };
    std::array dy = { 0, 1, 1,  1,  0, -1, -1, -1 };

	for (int i = 0; i < 8; ++i)
	{
		int nx = x + dx[i];
		int ny = y + dy[i];
		if (grid->is_burning(nx, ny))
			burnProbability += 0.5 + thread_rand() * 0.5;
	}

	return thread_rand() < p->flammability * burnProbability;
}

bool Simulation::dissolves(Particle* p, int x, int y)
{
	float dissolveProbability = 0;
	std::array dx = { 1, 1, 0, -1, -1, -1,  0,  1 };
	std::array dy = { 0, 1, 1,  1,  0, -1, -1, -1 };

	for (int i = 0; i < 8; ++i)
	{
		int nx = x + dx[i];
		int ny = y + dy[i];
		if (grid->is_liquid(nx, ny))
			dissolveProbability += 0.5 + thread_rand() * 0.5;
	}

	return thread_rand() < p->dissolvability * dissolveProbability;
}

bool Simulation::extinguishes(Particle* p, int x, int y)
{
	float extinguishProbability = 0;
	std::array dx = { 1, 1, 0, -1, -1, -1,  0,  1 };
	std::array dy = { 0, 1, 1,  1,  0, -1, -1, -1 };

	for (int i = 0; i < 8; ++i)
	{
		int nx = x + dx[i];
		int ny = y + dy[i];
		if (grid->is_extinguisher(nx, ny))
			extinguishProbability += 0.5 + thread_rand() * 0.5;
	}

	return thread_rand() < 0.5 * extinguishProbability;
}

void Simulation::sand(Particle* p, int x, int y)
{
	solid(p, x, y);
}

void Simulation::water(Particle* p, int x, int y)
{
	liquid(p, x, y);
}

void Simulation::wood(Particle* p, int x, int y)
{
	if (burns(p, x, y))
	{
		grid->set(x, y, Particle::FIRE);
		// TODO: customize burn time based on particle type
		grid->get(x, y)->life_time = 1.0 + thread_rand();
	}
}

void Simulation::smoke(Particle* p, int x, int y)
{
	if (p->life_time < 0.2f + thread_rand())
		p->burning = false;
	air(p, x, y);
}

void Simulation::fire(Particle* p, int x, int y)
{
	if (extinguishes(p, x, y))
	{
		// Liquid puts out fire
		grid->set(x, y, Particle::SMOKE);
	}
	else if (p->life_time < 0.1f + 0.1f * thread_rand())
	{
		// Become smoke
		grid->set(x, y, Particle::SMOKE);
		grid->get(x, y)->burning = true;
	}
}

void Simulation::salt(Particle* p, int x, int y)
{
	if (dissolves(p, x, y))
		p->dying = true;
	solid(p, x, y);
}

void Simulation::acid(Particle* p, int x, int y)
{
    std::array dx = { 0, -1, 1, -1, 1 };
    std::array dy = { 1, 1, 1, 0, 0 };

	for (int i = 0; i < 5; ++i)
	{
		int nx = x + dx[i];
		int ny = y + dy[i];
		if (grid->is_solid(nx, ny))
		{
			auto np = grid->get(nx, ny);
			if (np && thread_rand() < np->corrodibility)
			{
				grid->set(nx, ny, Particle::EMPTY);
				break;
			}
		}
	}

	if (dissolves(p, x, y))
		p->dying = true;

	liquid(p, x, y);
}

void Simulation::gasoline(Particle* p, int x, int y)
{
	if (burns(p, x, y))
	{
		grid->set(x, y, Particle::FIRE);
		// TODO: customize burn time based on particle type
		grid->get(x, y)->life_time = 1.0 + thread_rand();
	}
	liquid(p, x, y);
}

void Simulation::vine(Particle* p, int x, int y)
{
}