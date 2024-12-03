#pragma once
#include <SDL_render.h>
#include <DirectXMath.h>
#include <vector>

#include "grid.h"

using namespace DirectX;

class ParticleSelectorUI
{
	std::vector<SDL_FRect> rects, rects_outline;
	float padding, vertical_padding;
public:
	explicit ParticleSelectorUI(float horizontal_padding, float vertical_padding);
	bool render(SDL_Renderer* renderer, XMINT2 window_size, Particle::Type* selected, int mouse_x, int mouse_y);
};
