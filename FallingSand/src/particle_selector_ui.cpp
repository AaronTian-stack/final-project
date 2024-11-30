#include "particle_selector_ui.h"

#include <SDL_mouse.h>

#include "grid.h"

ParticleSelectorUI::ParticleSelectorUI(float horizontal_padding, float vertical_padding) : padding(horizontal_padding), vertical_padding(vertical_padding)
{
}

bool ParticleSelectorUI::render(SDL_Renderer* renderer, XMINT2 window_size, Particle::Type* selected, int mouse_x,
                                int mouse_y)
{
    // TODO: don't hard code icon size
    float base_icon_size = 30.f;
    float max_icon_size = 60.f;
    auto influence_radius = 80.f;

    float x = static_cast<float>(window_size.x) - padding - base_icon_size;
    float y = padding + vertical_padding;

    // assume 20 >= # of particle types
	// TODO: replace with struct for readability
    std::array<XMFLOAT2, 20> icon_size_offset;

    int index = 0;
    for (int i = 1; i < Particle::EMPTY; i <<= 1)
    {
        // Calculate distance from mouse to icon center
        const float icon_center_x = x + base_icon_size * 0.5f;
        const float icon_center_y = y + base_icon_size * 0.5f;
		const auto x_dif = mouse_x - icon_center_x;
		const auto y_dif = mouse_y - icon_center_y;
        const float distance2 = x_dif * x_dif + y_dif * y_dif;

        // Calculate icon size based on distance
        const float scale = std::max(0.0f, 1.0f - distance2 / (influence_radius * influence_radius));

        float icon_size = base_icon_size + scale * (max_icon_size - base_icon_size);
        float adjusted_x = x - (icon_size - base_icon_size);

		icon_size_offset[index++] = { icon_size, adjusted_x };

		y += base_icon_size + padding;
    }

	rects.clear();

    index = 0;
    y = padding + vertical_padding;
    for (int i = 1; i < Particle::EMPTY; i <<= 1)
    {
	    const auto& icon_so = icon_size_offset[index++];

		rects.push_back({ icon_so.y, y, icon_so.x, icon_so.x });

		y += icon_so.x + padding;
    }

	bool inside = false;
	for (int i = 0; i < rects.size(); ++i)
	{
        SDL_FPoint mouse_point = { static_cast<float>(mouse_x), static_cast<float>(mouse_y) };
        auto color = ParticleUtils::colors.at(static_cast<Particle::Type>(1 << i));
        if (SDL_PointInFRect(&mouse_point, &rects[i]))
        {
			inside = true;
            if (SDL_GetMouseState(&mouse_x, &mouse_y) & SDL_BUTTON(SDL_BUTTON_LEFT))
            {
                color = color.mix(Color(0, 0, 0), 0.5f);
                *selected = static_cast<Particle::Type>(1 << i);
            }
        }
		auto& rect = rects[i];
        SDL_SetRenderDrawColor(renderer, color.r(), color.g(), color.b(), 255);
        SDL_RenderFillRectF(renderer, &rect);
	}
    return inside;
}
