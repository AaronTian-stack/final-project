#pragma once

#include "grid.h"

class ImageLoader
{
	Grid* grid;
	void quantize_to_grid(unsigned char* image, int w, int h, int comp);
public:
	ImageLoader(Grid* grid);
	void open();
};

