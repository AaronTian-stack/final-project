#include "simulation.h"

void Simulation::SimulateBottomToTop(Grid& grid)
{
	// iterate from bottom to top
	for (int y = grid.get_height() - 1; y >= 0; --y)
	{
		for (int x = 0; x < grid.get_width(); ++x)
		{
			Particle& particle = grid.get(x, y);
			if (particle.id == SAND)
			{
				if (y < grid.get_height() - 1)
				{
					Particle& below = grid.get(x, y + 1);
					if (below.id == EMPTY)
					{
						grid.swap(x, y, x, y + 1);
					}
					else if (x > 0 && grid.get(x - 1, y + 1).id == EMPTY)
					{
						grid.swap(x, y, x - 1, y + 1);
					}
					else if (x < grid.get_width() - 1 && grid.get(x + 1, y + 1).id == EMPTY)
					{
						grid.swap(x, y, x + 1, y + 1);
					}
				}
			}
		}
	}
}
