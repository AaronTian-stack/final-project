#include "sdl_util.h"

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

void SDL_Util::update_texture_via_grid(BS::thread_pool& pool, uint32_t* pixel_data, Grid& grid, int width, int height, int pitch, float alpha)
{
	ZoneScoped;
	const float one_minus = 1.f - alpha;
	const BS::multi_future<void> loop_future = pool.submit_loop<unsigned int>(0, height * width,
		[&](const unsigned int i)
		{
			auto x = i % grid.get_width();
			auto y = i / grid.get_width();
			auto particle = grid.get(x, y);

			unsigned int x_r = x;
			unsigned int y_r = y;

#ifdef INTERPOLATE
			if (particle->type != Particle::EMPTY)
			{
				x_r = x * alpha + particle->prev_pos.x * one_minus;
				y_r = y * alpha + particle->prev_pos.y * one_minus;
			}
#endif

			pixel_data[y_r * (pitch / 4) + x_r] = particle->color.hex();
			
		});
	loop_future.wait();
}
