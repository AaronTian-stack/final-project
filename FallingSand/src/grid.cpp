#include "grid.h"

Grid::Grid(unsigned int width, unsigned int height, BS::synced_stream& sync_err) :
	width(width), height(height), sync_err(sync_err)
{
	this->grid = new Particle[static_cast<size_t>(width * height)];
}

Grid::~Grid()
{
	delete[] grid;
}

Particle* Grid::get(int x, int y) const
{
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
#ifdef DEBUG
		sync_err.println("GET Out of range: ", x, ", ", y);
#endif
		return nullptr;

	}
	return &grid[y * width + x];
}

void Grid::set(int x, int y, Particle::Type particle_type)
{
	if (x < 0 || x >= width || y < 0 || y >= height)
	{
#ifdef DEBUG
		sync_err.println("SET Out of range: ", x, ", ", y);
#endif
		return;
	}

	Particle p;
	p.type = particle_type;
	p.color = ParticleUtils::colors.at(particle_type);
#ifdef INTERPOLATE
	p.prev_pos = { x, y };
#endif

	switch (particle_type)
	{
	case Particle::SAND:
		p.density = 100.f;
		p.corrodibility = 0.01f;
		p.color = Color_Util::vary_color(p.color);
		break;
	case Particle::WATER:
		p.density = 50.f;
		break;
	case Particle::STONE:
		p.density = 500.f;
		break;
	case Particle::WOOD:
		p.density = 200.f;
		p.flammability = 0.2f;
		p.corrodibility = 0.05f;
		p.color = Color_Util::vary_color(p.color);
		break;
	case Particle::SMOKE:
		p.life_time = 0.05f + 2.0f * thread_rand();
		p.density = 1.f;
		p.dying = true;
		p.color = Color_Util::vary_color(p.color);
		break;
	case Particle::FIRE:
		p.life_time = 0.2f + 0.1f * thread_rand();
		p.density = 2.f;
		p.burning = true;
		p.dying = true;
		p.color = Color_Util::vary_color(p.color);
		break;
	case Particle::SALT:
		p.life_time = 0.5f + 1.5f * thread_rand();
		p.density = 100.f;
		p.dissolvability = 0.05f;
		p.corrodibility = 0.15f;
		p.color = Color_Util::vary_color(p.color);
		break;
	case Particle::ACID:
		p.life_time = 5.0f + 5.0f * thread_rand();
		p.density = 60.f;
		p.dissolvability = 0.005f;
		break;
	case Particle::GASOLINE:
		p.density = 25.f;
		p.flammability = 0.15f;
		break;
	case Particle::VIRUS:
		p.life_time = 1.0f + 1.0f * thread_rand();
		p.density = 150;
		p.flammability = 0.5f;
		p.dissolvability = 0.02f;
		p.corrodibility = 0.1f;
		p.diffusibility = 0.03f;
		p.dying = true;
		p.color = Color_Util::vary_color(p.color);
		break;
	case Particle::POISON:
		p.density = 55.f;
		p.diffusibility = 0.01f + 0.02f * thread_rand();
		break;
	default:
		break;
	}

	grid[y * width + x] = p;
}

void Grid::swap(int x1, int y1, int x2, int y2)
{
	if (!is_valid(x1, y1) || !is_valid(x2, y2)) return;
	auto xy1 = get(x1, y1);
	auto xy2 = get(x2, y2);
	if (!xy1 || !xy2) return;
	Particle temp = *xy1;
	grid[y1 * width + x1] = *xy2;
	grid[y2 * width + x2] = temp;
}

Particle::Type Grid::get_type(int x, int y)
{
	// TODO: better default value
	if (!is_valid(x, y)) return Particle::EMPTY;
	return get(x, y)->type;
}

bool Grid::is_valid(int x, int y) const
{
	return x >= 0 && x < width && y >= 0 && y < height;
}

bool Grid::is_air(int x, int y) const
{
	if (!is_valid(x, y)) return false;
	return ParticleUtils::is_air(get(x, y)->type);
}

bool Grid::is_liquid(int x, int y) const
{
	if (!is_valid(x, y)) return false;
	return ParticleUtils::is_liquid(get(x, y)->type);
}

bool Grid::is_solid(int x, int y) const
{
	if (!is_valid(x, y)) return false;
	return ParticleUtils::is_solid(get(x, y)->type);
}

bool Grid::is_burning(int x, int y) const
{
	if (!is_valid(x, y)) return false;
	return get(x, y)->burning;
}

bool Grid::is_extinguisher(int x, int y) const
{
	if (!is_valid(x, y)) return false;
	return get(x, y)->type & Particle::WATER;
}

bool Grid::is_denser(Particle* particle, int x, int y) const
{
	if (!is_valid(x, y)) return false; // assume out of bounds infinitely dense
	return get(x, y)->density < particle->density;
}