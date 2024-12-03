#pragma once

#include "grid.h"

class ImageLoader
{
	inline static std::vector<std::pair<Particle::Type, Color>> color_palette;
	Grid* grid;
	void quantize_to_grid(unsigned char* image, int w, int h, int channels);
public:
	ImageLoader(Grid* grid);
	void open();
};

