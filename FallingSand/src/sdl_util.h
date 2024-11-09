#pragma once
#include <SDL_render.h>

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
};
