#include "image_loader.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_STATIC
#include "stb_image_resize2.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define NOMINMAX
#include <windows.h>
#include <commdlg.h>

ImageLoader::ImageLoader(Grid* grid) : grid(grid)
{
	for (const auto& [type, color] : ParticleUtils::colors)
		color_palette.emplace_back(type, color);
}

// Source: https://www.compuphase.com/cmetric.htm
float red_mean_dist(Color c1, Color c2)
{
	long rmean = (c1.r() + c2.r()) / 2;
	long dr = (long)c1.r() - (long)c2.r();
	long dg = (long)c1.g() - (long)c2.g();
	long db = (long)c1.b() - (long)c2.b();

	long rcomp = ((512 + rmean) * dr * dr) >> 8;
	long gcomp = 4 * dg * dg;
	long bcomp = ((767 - rmean) * db * db) >> 8;

	return std::sqrtf(rcomp + gcomp + bcomp);
}

void ImageLoader::quantize_to_grid(unsigned char* image, int w, int h, int channels)
{
	for (int x = 0; x < w; ++x)
	{
		for (int y = 0; y < h; ++y)
		{
			int offset = y * w * channels + x * channels;
			auto color = Color(image[offset], image[offset + 1], image[offset + 2]);

			auto min_delta = std::numeric_limits<float>::max();
			Particle::Type particle_type = Particle::EMPTY;

			for (Particle::Type pt : ParticleUtils::quantize_palette)
			{
				Color pc = ParticleUtils::colors.at(pt);
				auto d = red_mean_dist(color, pc);
				if (d < min_delta)
				{
					min_delta = d;
					particle_type = pt;
				}
			}

			grid->set(x, y, particle_type);
		}
	}
}

void ImageLoader::open()
{
	OPENFILENAME ofn;       // Common dialog box structure
	char szFile[260];		// Buffer for file name

	// Initialize OPENFILENAME
	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = NULL;
	ofn.lpstrFile = szFile;
	ofn.lpstrFile[0] = '\0';
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = "Image files (*.jpg;*.png;*.bmp)\0*.jpg;*.png;*.bmp\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	int w;
	int h;
	int channels;
	unsigned char* raw_image;

	// Load the image
	if (GetOpenFileName(&ofn) == TRUE)
	{
		stbi_info(ofn.lpstrFile, &w, &h, &channels);
		auto req_comp = STBI_rgb;
		switch (channels)
		{
		case 1:
			req_comp = STBI_grey;
			break;
		case 3:
			req_comp = STBI_rgb;
			break;
		case 4:
			req_comp = STBI_rgb_alpha;
			break;
		default:
			std::cerr << "Unsupported number of channels: " << channels << std::endl;
		}

		raw_image = stbi_load(ofn.lpstrFile, &w, &h, &channels, req_comp);

		if (raw_image == nullptr)
		{
			std::cerr << "Failed to load image: " << ofn.lpstrFile << std::endl;
			return;
		}
	}
	else
	{
		std::cerr << "Failed to open file: " << ofn.lpstrFile << std::endl;
		return;
	}

	// Resize image to grid
	auto grid_w = static_cast<int>(grid->get_width());
	auto grid_h = static_cast<int>(grid->get_height());

	auto layout = STBIR_1CHANNEL;
	bool can_resize = true;
	switch (channels)
	{
	case 1:
		layout = STBIR_1CHANNEL;
		break;
	case 3:
		layout = STBIR_RGB;
		break;
	case 4:
		layout = STBIR_RGBA;
		break;
	default:
		can_resize = false;
		std::cerr << "Unsupported number of channels: " << channels << std::endl;
	}

	unsigned char* resized_image = nullptr;
	if (can_resize)
		resized_image = stbir_resize_uint8_srgb(raw_image, w, h, 0, nullptr, grid_w, grid_h, 0, layout);

	if (resized_image)
	{
		quantize_to_grid(resized_image, grid_w, grid_h, channels);
	}
	else
	{
		std::cerr << "Failed to resize image" << std::endl;
	}

	stbi_image_free(raw_image);
	free(resized_image);
}