#include "image_upload_ui.h"

#include <SDL_image.h>

ImageUploadUI::ImageUploadUI(SDL_Renderer* renderer, const char* img_path, float padding) : padding(padding)
{
	img = IMG_LoadTexture(renderer, img_path);
	SDL_QueryTexture(img, NULL, NULL, &w, &h);
}

bool ImageUploadUI::render(SDL_Renderer* renderer, ImageLoader loader, XMINT2 window_size, int mouse_x, int mouse_y)
{
	SDL_FRect tex
	{
		.x = padding,
		.y = padding,
		.w = static_cast<float>(w) * 0.1f,
		.h = static_cast<float>(h) * 0.1f
	};

	bool ret = false;
	// if mouse is hovering grow the icon
	SDL_FPoint mouse = { static_cast<float>(mouse_x), static_cast<float>(mouse_y) };
	if (SDL_PointInFRect(&mouse, &tex))
	{
		tex.x -= tex.w * 0.1f;
		tex.y -= tex.h * 0.1f;
		tex.w *= 1.2f;
		tex.h *= 1.2f;
		ret = true;
		if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_LEFT))
		{
			loader.open();
		}
	}

	SDL_RenderCopyF(renderer, img, NULL, &tex);

	return ret;
}
