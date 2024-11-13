#pragma once
#include <random>

#include "particle.h"

// make child class for different brush patterns
class Brush
{
protected:
	int brush_size;

public:
	explicit Brush(int brush_size);

	// brush pattern function
	virtual bool should_draw(int local_x, int local_y) = 0;
	void draw_particles(Grid& grid, ParticleType particle_type);
	int get_brush_size() const { return brush_size; }
	void set_brush_size(int size) { brush_size = size; }

	virtual ~Brush() = default;
};

class CircleBrush : public Brush
{
	public:
	explicit CircleBrush(int brush_size);
	bool should_draw(int local_x, int local_y) override;
};

class RandomBrush : public Brush
{
	std::random_device rd;
	std::mt19937 mt;
	std::uniform_real_distribution<float> dist;
	float prob;
public:
	explicit RandomBrush(int brush_size, float prob);
	bool should_draw(int local_x, int local_y) override;
};
