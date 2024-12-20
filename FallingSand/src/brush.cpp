﻿#include "brush.h"

#include <iostream>
#include <SDL.h>

#include "grid.h"
#include "color.h"

Brush::Brush(int brush_size) : brush_size(brush_size)
{
}

void Brush::draw_particles(Grid& grid, Particle::Type particle_type, XMFLOAT2 velocity)
{
	int mouseX, mouseY;
	// when clicked on grid, set particle
	auto left_click = SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_LEFT);
	auto right_click = SDL_GetMouseState(&mouseX, &mouseY) & SDL_BUTTON(SDL_BUTTON_RIGHT);
	if (left_click || right_click)
	{
		if (right_click) particle_type = Particle::EMPTY;
		// set all pixels within brush size to particle
		for (int y = mouseY - brush_size; y < mouseY + brush_size; ++y)
		{
			auto local_y = y - mouseY;
			for (int x = mouseX - brush_size; x < mouseX + brush_size; ++x)
			{
				auto x2 = x - mouseX;
				x2 = x2 * x2;
				auto y2 = y - mouseY;
				y2 = y2 * y2;

				auto local_x = x - mouseX;

				if (x2 + y2 < brush_size * brush_size && should_draw(local_x, local_y))
				{
					grid.set(x, y, particle_type);
					//grid.get(x, y)->velocity = velocity;
				}
			}
		}
	}
}

CircleBrush::CircleBrush(int brush_size) : Brush(brush_size)
{
}

bool CircleBrush::should_draw(int local_x, int local_y)
{
	return true;
}

RandomBrush::RandomBrush(int brush_size, float prob) : Brush(brush_size), mt(rd()), dist(0, 1), prob(prob)
{
}

bool RandomBrush::should_draw(int local_x, int local_y)
{
	return dist(mt) < prob;
}
