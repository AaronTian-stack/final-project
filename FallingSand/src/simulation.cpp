#include "simulation.h"

#include <tbb/tbb.h>

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
			return { prev_x, prev_y };

		prev_x = x;
		prev_y = y;
	}
}

void Simulation::simulate(float delta)
{
	const int num_columns = std::thread::hardware_concurrency() * 2;
	auto pixels_per_group = grid->get_width() / num_columns;
	auto step = pixels_per_group * 2;

	tbb::task_group tg;

	auto process_columns = [this, delta, step](int start)
		{
			tbb::parallel_for(tbb::blocked_range<int>(start, grid->get_width(), step),
				[this, delta](const tbb::blocked_range<int>& range)
				{
					for (int x = range.begin(); x < range.end(); x++)
					{
						for (int y = grid->get_height() - 1; y >= 0; --y)
						{
							auto particle = grid->get(x, y);

							if (!ParticleUtils::reversed_simulation(particle->type))
							{
								if (ParticleUtils::affected_by_gravity(particle->type))
								{
									if (grid->is_denser(particle, x, y + 1))
										particle->velocity.y += gravity * delta;

									int vx = dist(mt) < 0.5f ? ceil(particle->velocity.x) : floor(particle->velocity.x);
									int vy = dist(mt) < 0.5f ? ceil(particle->velocity.y) : floor(particle->velocity.y);

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
							default:
								break;
							}
						}
					}
				});
		};

	auto process_rising_columns = [this, delta, step](int start)
		{
			tbb::parallel_for(tbb::blocked_range<int>(start, grid->get_width(), step),
				[this, delta](const tbb::blocked_range<int>& range)
				{
					for (int x = range.begin(); x < range.end(); x++)
					{
						for (int y = 0; y < grid->get_height(); ++y)
						{
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
				});
		};

	tg.run([&] { process_columns(0); });
	tg.run([&] { process_rising_columns(0); });

	tg.wait();

	tg.run([&] { process_columns(1); });
	tg.run([&] { process_rising_columns(1); });

	tg.wait();
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
			burnProbability += 0.5 + dist(mt) * 0.5;
	}

	return dist(mt) < p->flammability * burnProbability;
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
			dissolveProbability += 0.5 + dist(mt) * 0.5;
	}

	return dist(mt) < p->dissolvability * dissolveProbability;
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
		grid->get(x, y)->life_time = 1.0 + dist(mt);
	}
}

void Simulation::smoke(Particle* p, int x, int y)
{
	if (p->life_time < 0.2f + dist(mt))
		p->burning = false;
	air(p, x, y);
}

void Simulation::fire(Particle* p, int x, int y)
{
	if (dissolves(p, x, y))
	{
		// Liquid puts out fire
		grid->set(x, y, Particle::SMOKE);
	}
	else if (p->life_time < 0.1f + 0.1f * dist(mt))
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
			if (np && dist(mt) < np->corrodibility)
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