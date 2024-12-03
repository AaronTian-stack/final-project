#include <stdexcept>

#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL.h>

#include "brush.h"
#include "sdl_util.h"
#include "simulation.h"

#include <Tracy.hpp>

#include "image_loader.h"
#include "image_upload_ui.h"
#include "particle_selector_ui.h"

int main()
{
	//*** REMOVE TRACY_ENABLE FROM PREPROCESSOR DEFINITION ON REAL RELEASE OR ELSE MEMORY WILL KEEP GROWING ***//

	const int WIDTH = 1280;
	const int HEIGHT = 720;

	if (SDL_Init(SDL_INIT_VIDEO) != 0)
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

	BS::synced_stream sync_err(std::cerr);
	BS::thread_pool pool;

	Grid grid(WIDTH, HEIGHT, sync_err);
	int brush_size = 10;
	CircleBrush circle_brush(brush_size);
	RandomBrush rand_brush(brush_size, 0.1f);
	Simulation simulation(&grid);

	Particle::Type selected_particle = Particle::SAND;

	ImageLoader image_loader(&grid);
	ImageUploadUI image_upload_ui(renderer, "assets/upload.png", 20.f);
	ParticleSelectorUI particle_selector_ui(10, 40);

	auto update_brush_radii = [&brush_size, &circle_brush, &rand_brush](int size)
	{
		brush_size = std::clamp(size, 1, 100);
		circle_brush.set_brush_size(brush_size);
		rand_brush.set_brush_size(brush_size);
	};

	bool quit = false;
	bool over_UI = false;
	float delta = 0.f;
	float accum = 0.f;
	constexpr float dt = 1.f / 30.f;
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
				case SDLK_ESCAPE:
					quit = true;
					break;
				case SDLK_LEFT:
					selected_particle = static_cast<Particle::Type>(std::max(selected_particle / 2, static_cast<int>(Particle::SAND)));
					break;
				case SDLK_RIGHT:
					selected_particle = static_cast<Particle::Type>(std::max(selected_particle * 2 % Particle::EMPTY, static_cast<int>(Particle::SAND)));
					break;
				// TODO: create UI for triggering open()
				case SDLK_SPACE:
					image_loader.open();
					break;
				case SDLK_a:
					update_brush_radii(brush_size - 1);
					break;
				case SDLK_d:
					update_brush_radii(brush_size + 1);
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
		static auto start_timer = std::chrono::high_resolution_clock::now();

		accum += delta;
		while (accum > dt)
		{
			simulation.update(dt, pool);
			accum -= dt;
		}
		float alpha = accum / dt;
		
		// click to draw
		// TODO: customize brush
		if (!over_UI)
		{
			if (ParticleUtils::use_solid_brush(selected_particle))
				circle_brush.draw_particles(grid, selected_particle);
			else
				rand_brush.draw_particles(grid, selected_particle); // can set default velocity
		}

		// RENDER
		void* pixels;
		int pitch;
		if (SDL_LockTexture(texture, nullptr, &pixels, &pitch) != 0)
		{
			SDL_Log("Unable to lock texture: %s", SDL_GetError());
			break;
		}

		auto pixel_data = static_cast<uint32_t*>(pixels);

		SDL_Util::update_texture_via_grid(pool, pixel_data, grid, WIDTH, HEIGHT, pitch, alpha);

		int mouse_x, mouse_y;
		SDL_GetMouseState(&mouse_x, &mouse_y);
		Color mouseColor = ParticleUtils::colors.at(selected_particle);
		if (!over_UI)
		{
			SDL_Util::draw_circle(
				{
					.pixelData = pixel_data,
					.width = WIDTH,
					.height = HEIGHT
				},
				mouse_x, mouse_y, rand_brush.get_brush_size(), mouseColor.hex());
		}

		SDL_UnlockTexture(texture);

		// present
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, texture, nullptr, nullptr);

		auto over_particle = particle_selector_ui.render(renderer, { WIDTH, HEIGHT }, &selected_particle, mouse_x, mouse_y);
		auto over_image = image_upload_ui.render(renderer, image_loader, { WIDTH, HEIGHT }, mouse_x, mouse_y);
		over_UI = over_particle || over_image;

		SDL_RenderPresent(renderer);

		static auto end_timer = std::chrono::high_resolution_clock::now();
		delta = std::chrono::duration<float, std::milli>(end_timer - start_timer).count() / 1000.f;

		FrameMark;
	}

	SDL_DestroyTexture(texture);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}
