#pragma once
#include "grid.h"

class Simulation
{
public:
	// Only particles with gravity can be simulated bottom to top. Would not work with smoke for example
	static void SimulateBottomToTop(Grid& grid);
	// TODO: insert logic to update individual particles

};
