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
	// Slows down at 1920x1000 (8 ms simulation)
	const int WIDTH = 1920;
	const int HEIGHT = 1000;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
	{
		SDL_Log("Unable to initialize SDL: %s", SDL_GetError());
		throw std::runtime_error("Unable to initialize SDL");
	}

	SDL_Window* window = SDL_CreateWindow("CIS 5660 | Falling Sand", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
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

	int brush_width = 10;

	Grid grid(WIDTH, HEIGHT);
	CircleBrush circle_brush(brush_width);
	RandomBrush rand_brush(brush_width, 0.02f);
	Simulation simulation(&grid);

	Particle::Type curr_particle = Particle::SAND;

	bool quit = false;
	double delta = 0.f;
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
				switch (event.key.keysym.sym)
				{
			case SDLK_LEFT:
				curr_particle = static_cast<Particle::Type>(std::max(curr_particle / 2, static_cast<int>(Particle::SAND)));
					break;
			case SDLK_RIGHT:
					curr_particle = static_cast<Particle::Type>(std::max(curr_particle * 2 % Particle::EMPTY, static_cast<int>(Particle::SAND)));
					break;
				default:
					break;
				}
				break;
			default:
				break;
			}
		}
		// start timer
		static Uint64 last_time = SDL_GetTicks64();
		simulation.update(delta);
		static Uint64 current_time = SDL_GetTicks64();
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
		// TODO: customize brush
		if (ParticleUtils::use_solid_brush(curr_particle))
			circle_brush.draw_particles(grid, curr_particle);
		else
			rand_brush.draw_particles(grid, curr_particle);

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
		Color mouseColor = ParticleUtils::colors.at(curr_particle);
		SDL_Util::draw_circle(
			{
				.pixelData = pixelData,
				.width = WIDTH,
				.height = HEIGHT
			}, 
			mouseX, mouseY, rand_brush.get_brush_size(), mouseColor.hex());

		SDL_UnlockTexture(texture);

		// present
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);
		SDL_RenderPresent(renderer);

		static Uint64 frame_time = SDL_GetTicks64();
		delta = (frame_time - last_time) / 1000.f;
#ifdef NDEBUG
		std::cout << "Delta time: " << delta * 1000 << " ms" << std::endl;
#endif

		//SDL_Delay(16); // wait 16 ms (frame lock)
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
