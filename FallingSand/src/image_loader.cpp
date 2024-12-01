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
}

// Source: https://www.compuphase.com/cmetric.htm
double red_mean_dist(Color c1, Color c2)
{
	long rmean = (c1.r() + c2.r()) / 2;
	long dr = (long)c1.r() - (long)c2.r();
	long dg = (long)c1.g() - (long)c2.g();
	long db = (long)c1.b() - (long)c2.b();

	long rcomp = ((512 + rmean) * dr * dr) >> 8;
	long gcomp = 4 * dg * dg;
	long bcomp = ((767 - rmean) * db * db) >> 8;

	return std::sqrt(rcomp + gcomp + bcomp);
}

void ImageLoader::quantize_to_grid(unsigned char* image, int w, int h, int comp)
{
	for (int x = 0; x < w; ++x)
	{
		for (int y = 0; y < h; ++y)
		{
			int offset = y * w * comp + x * comp;
			Color color = Color(image[offset], image[offset + 1], image[offset + 2]);

			double min_delta = std::numeric_limits<float>::max();
			Particle::Type particle_type = Particle::EMPTY;

			for (Particle::Type pt : ParticleUtils::quantize_palette)
			{
				Color particle_color = ParticleUtils::colors.at(pt);
				double d = red_mean_dist(color, particle_color);
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
	int comp;
	unsigned char* raw_image;

	// Load the image
	if (GetOpenFileName(&ofn) == TRUE)
	{
		raw_image = stbi_load(ofn.lpstrFile, &w, &h, &comp, STBI_rgb);

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
	int grid_w = grid->get_width();
	int grid_h = grid->get_height();
	unsigned char* resized_image = (unsigned char*) malloc(grid_w * grid_h * comp);

	stbir_resize_uint8_srgb(raw_image, w, h, 0, resized_image, grid_w, grid_h, 0, STBIR_RGB);

	// Color quantization
	if (resized_image != nullptr)
	{
		quantize_to_grid(resized_image, grid_w, grid_h, comp);
	}
	else
	{
		std::cerr << "Failed to resize image: " << ofn.lpstrFile << std::endl;
	}

	// Free data
	stbi_image_free(raw_image);
	free(resized_image);
}