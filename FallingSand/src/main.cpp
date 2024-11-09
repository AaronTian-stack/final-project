#include <stdexcept>

#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL.h>

#include "sdl_util.h"
#include "simulation.h"

int main()
{
	// TODO: use SDL to draw UI, debugging grids/lines...
	const int WIDTH = 800;
	const int HEIGHT = 600;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		throw std::runtime_error("Unable to initialize SDL");
	}

	SDL_Window* window = SDL_CreateWindow("CIS 5660 | Falling Sand", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 600, SDL_WINDOW_SHOWN);
	if (window == nullptr)
	{
		SDL_Log("Unable to create window: %s", SDL_GetError());
		SDL_Quit();
		throw std::runtime_error("Unable to create window");
	}

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (renderer == nullptr)
	{
		SDL_Log("Unable to create renderer: %s", SDL_GetError());
		SDL_DestroyWindow(window);
		SDL_Quit();
		throw std::runtime_error("Unable to create renderer");
	}

	// maybe move into grid
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, WIDTH, HEIGHT);
	if (texture == nullptr)
	{
		SDL_Log("Unable to create texture: %s", SDL_GetError());
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		SDL_Quit();
		throw std::runtime_error("Unable to create texture");
	}

	Grid grid(WIDTH, HEIGHT);
	int brush_size = 10;

	const int NUM_PARTICLES = 2;
	const Particle PARTICLES[NUM_PARTICLES] = {
		{ SAND, 0, 0, {0, 0}, 0xF4A460, 1 },
		{ STONE, 0, 0, {0, 0}, 0x888888, 0 }
	};
	int particle = 0;

	bool quit = false;
	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			switch (event.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_KEYDOWN:
				particle = (particle + 1) % NUM_PARTICLES;
				break;
			default:
				break;
			}
		}

		Simulation::SimulateBottomToTop(grid);

		// RENDER
		void* pixels;
		int pitch;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0)
		{
			SDL_Log("Unable to lock texture: %s", SDL_GetError());
			break;
		}

		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);
		// when clicked on grid, set particle
		if (SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			// set all pixels within brush size to particle
			for (int y = mouseY - brush_size; y < mouseY + brush_size; ++y)
			{
				for (int x = mouseX - brush_size; x < mouseX + brush_size; ++x)
				{
					auto x2 = x - mouseX;
					x2 = x2 * x2;
					auto y2 = y - mouseY;
					y2 = y2 * y2;
					if (0 <= x && x < WIDTH && 0 <= y && y < HEIGHT && x2 + y2 < brush_size * brush_size)
					{
						Particle p = PARTICLES[particle];
						if (grid.isEmpty(x, y) || p.priority < grid.get(x, y).priority)
						{
							grid.set(x, y, PARTICLES[particle]);
						}
					}
				}
			}
		}

		uint32_t* pixelData = static_cast<uint32_t*>(pixels);
		for (int y = 0; y < HEIGHT; ++y)
		{
			for (int x = 0; x < WIDTH; ++x)
			{
				auto& particle = grid.get(x, y);
				pixelData[y * (pitch / 4) + x] = particle.color.hex();
			}
		}
		SDL_Util::draw_circle(
			{
				.pixelData = pixelData,
				.width = WIDTH,
				.height = HEIGHT
			}, 
			mouseX, mouseY, brush_size, 0xFFFFFF);

		SDL_UnlockTexture(texture);

		// present
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		//SDL_Delay(16); // wait 16 ms (frame lock)
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
