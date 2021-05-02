#pragma once

#include <SDL2/unique_resource.h>

namespace omm
{
	class program
	{
		sdl::unique_window window;
		sdl::unique_renderer renderer;

		int initialize_window(int argc, char** argv);

	public:
		int run(int argc, char** argv);
	};
}
