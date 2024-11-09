#include <stdexcept>

#define SDL_MAIN_HANDLED
#include <SDL.h>

int main()
{
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

		// render
	}
	return 0;
}
