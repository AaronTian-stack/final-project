#include "sdl_util.h"

#include <cmath>

void SDL_Util::put_pixel(const CanvasInfo& info, int x, int y, uint32_t color)
{
	if (x < 0 || x >= info.width || y < 0 || y >= info.height)
	{
		return;
	}
	info.pixelData[y * info.width + x] = color;
}

void SDL_Util::draw_circle(const CanvasInfo& info, int c_x, int c_y, int radius, uint32_t color)
{
	auto points = [&](int xc, int yc, int x, int y)
	{
		put_pixel(info, xc + x, yc + y, color);
		put_pixel(info, xc - x, yc + y, color);
		put_pixel(info, xc + x, yc - y, color);
		put_pixel(info, xc - x, yc - y, color);
		put_pixel(info, xc + y, yc + x, color);
		put_pixel(info, xc - y, yc + x, color);
		put_pixel(info, xc + y, yc - x, color);
		put_pixel(info, xc - y, yc - x, color);
	};

	int x = 0, y = radius;
	int d = 3 - 2 * radius;
	points(c_x, c_y, x, y);
	while (y >= x) {

		if (d > 0) {
			y--;
			d = d + 4 * (x - y) + 10;
		}
		else
			d = d + 4 * x + 6;

		x++;
		points(c_x, c_y, x, y);
	}
}
