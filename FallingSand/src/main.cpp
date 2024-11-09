#include <stdexcept>

#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL.h>

#include "brush.h"
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
	RandomBrush brush(10, 0.4f);
	Simulation simulation(&grid);

	bool quit = false;
	while (!quit)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				quit = true;
			}
		}
		// start timer
		static Uint32 last_time = SDL_GetTicks64();
		simulation.update();
		static Uint32 current_time = SDL_GetTicks64();
#ifdef NDEBUG
		std::cout << "Simulation time: " << current_time - last_time << " ms" << std::endl;
#endif

		// RENDER
		void* pixels;
		int pitch;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0)
		{
			SDL_Log("Unable to lock texture: %s", SDL_GetError());
			break;
		}

		// click to draw
		brush.draw_particles(grid, Particle::SAND);

		uint32_t* pixelData = static_cast<uint32_t*>(pixels);
		for (int y = 0; y < HEIGHT; ++y)
		{
			for (int x = 0; x < WIDTH; ++x)
			{
				auto particle = grid.get(x, y);
				pixelData[y * (pitch / 4) + x] = particle->color.hex();
			}
		}
		int mouseX, mouseY;
		SDL_GetMouseState(&mouseX, &mouseY);
		SDL_Util::draw_circle(
			{
				.pixelData = pixelData,
				.width = WIDTH,
				.height = HEIGHT
			}, 
			mouseX, mouseY, brush.get_brush_size(), 0xFFFFFF);

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
