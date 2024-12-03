#pragma once

#include <BS_thread_pool.hpp>

#include "grid.h"
#include <Tracy.hpp>

struct CanvasInfo
{
	uint32_t* pixelData;
	int width;
	int height;
};

class SDL_Util
{
public:
	static void put_pixel(const CanvasInfo& info, int x, int y, uint32_t color);
	static void draw_circle(const CanvasInfo& info, int c_x, int c_y, int radius, uint32_t color);
	// called after you lock the texture
	static void update_texture_via_grid(BS::thread_pool& pool, uint32_t* pixel_data, Grid& grid, int width, int height, int pitch, float alpha);
};
