#pragma once
#include <DirectXMath.h>
#include <SDL_render.h>

#include "image_loader.h"

using namespace DirectX;

class ImageUploadUI
{
	float padding;
	int w, h;
	SDL_Texture* img;
public:
	ImageUploadUI(SDL_Renderer* renderer, const char* img_path, float padding);
	bool render(SDL_Renderer* renderer, ImageLoader loader, XMINT2 window_size, int mouse_x, int mouse_y);
};
