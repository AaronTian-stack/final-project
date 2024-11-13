#include "particle.h"
#include <random>

Particle::Particle()
{
}

void Particle::simulate_step(Grid* grid, int x, int y)
{
}

Sand::Sand()
{
	color = Color(0xFFD700);
	color = Color_Util::vary_color(color);
	density = 100;
	is_air = false;
	has_gravity = true;
}

void Sand::simulate_step(Grid* grid, int x, int y)
{
	if (grid->is_denser(*this, x, y + 1))
	{
		grid->swap(x, y, x, y + 1);
	}
	else if (grid->is_denser(*this, x - 1, y + 1))
	{
		grid->swap(x, y, x - 1, y + 1);
	}
	else if (grid->is_denser(*this, x + 1, y + 1))
	{
		grid->swap(x, y, x + 1, y + 1);
	}
	else
	{
		velocity.y = 0;
	}
}

Water::Water()
{
	color = Color(0x0000FF);
	density = 50;
	is_air = false;
	has_gravity = true;
}

void Water::simulate_step(Grid* grid, int x, int y)
{
	if (grid->is_denser(*this, x, y + 1))
	{
		grid->swap(x, y, x, y + 1);
	}
	else if (grid->is_denser(*this, x - 1, y + 1))
	{
		grid->swap(x, y, x - 1, y + 1);
	}
	else if (grid->is_denser(*this, x + 1, y + 1))
	{
		grid->swap(x, y, x + 1, y + 1);
	}
	else if (grid->is_denser(*this, x - 1, y))
	{
		grid->swap(x, y, x - 1, y);
	}
	else if (grid->is_denser(*this, x + 1, y))
	{
		grid->swap(x, y, x + 1, y);
	}
	else
	{
		velocity.y = 0;
	}
}

Wood::Wood()
{
	color = Color(0x362312);
	flammability = 0.5;
	density = 200;
	is_air = false;
}

Smoke::Smoke()
{
	color = Color(0x888888);
	color = Color_Util::vary_color(color);
	life_time = 2.0 + ((double) rand()) / RAND_MAX;
	density = 1;
	simulate_reverse = true;
}

void Smoke::simulate_step(Grid* grid, int x, int y)
{
	if (life_time < 0)
	{
		// TODO: decrease alpha with lifetime
		grid->set(x, y, Empty());
		return;
	}
	if (grid->is_denser(*this, x, y - 1))
	{
		grid->swap(x, y, x, y - 1);
	}
	else if (grid->is_denser(*this, x - 1, y - 1))
	{
		grid->swap(x, y, x - 1, y - 1);
	}
	else if (grid->is_denser(*this, x + 1, y - 1))
	{
		grid->swap(x, y, x + 1, y - 1);
	}
}

Fire::Fire()
{
	color = Color(0xFF4500);
	color = Color_Util::vary_color(color);
	life_time = 3.0 + 2 * ((double) rand()) / RAND_MAX;
	density = 2;
	simulate_reverse = true;
}

void Fire::simulate_step(Grid* grid, int x, int y)
{
	if (life_time < 0)
	{
		grid->set(x, y, Smoke());
		return;
	}

	int dx[] = { 1, 1, 0, -1, -1, -1,  0,  1 };
	int dy[] = { 0, 1, 1,  1,  0, -1, -1, -1 };

	for (int i = 0; i < 8; ++i)
	{
		int nx = x + dx[i];
		int ny = y + dy[i];
		if (grid->catches_fire(nx, ny))
			grid->set(nx, ny, Fire());
	}
}