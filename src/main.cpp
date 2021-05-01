#include <SDL.h>

#include <SDL2/macro.h>
#include <SDL2/unique_resource.h>

#include <string>
#include <stdexcept>

namespace omm
{
	namespace
	{
		int get_direct3d11_index()
		{
			int const num_drivers = SDL_GetNumRenderDrivers();
			for (int i = 0; i < num_drivers; ++i)
			{
				SDL_RendererInfo info;
				SDL_GetRenderDriverInfo(i, &info);

				if (strcmp(info.name, "direct3d11") == 0)
					return i;
			}

			throw std::runtime_error("Direct3d11 render driver could not be found");
		}

		class program
		{
			sdl::unique_window window;
			sdl::unique_renderer renderer;

			int initialize_window(int argc, char** argv);

		public:
			int run(int argc, char** argv);
		};

		int program::initialize_window(int argc, char** argv)
		{
			window.reset(SDL_CreateWindow("OpenMM3", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080, SDL_WINDOW_BORDERLESS));
			if (window == nullptr)
			{
				SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create SDL window: %s", SDL_GetError());
				return -1;
			}

			renderer.reset(SDL_CreateRenderer(window.get(), get_direct3d11_index(), SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC));
			if (renderer == nullptr)
			{
				SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not create SDL renderer: %s", SDL_GetError());
				return -1;
			}

			return 0;
		}

		int program::run(int argc, char** argv)
		{
			if (int result = initialize_window(argc, argv))
			{
				return result;
			}

			while(true)
			{
				SDL_PumpEvents();

				SDL_Event e;
				while (SDL_PollEvent(&e))
				{
					switch (e.type)
					{
					case SDL_QUIT:
						return 0;
					}
				}

				OMM_SDL_FAILURE_IF(SDL_SetRenderDrawColor(renderer.get(), 148, 155, 203, SDL_ALPHA_OPAQUE));
				OMM_SDL_FAILURE_IF(SDL_RenderClear(renderer.get()));
				SDL_RenderPresent(renderer.get());
			}
		}
	}
}

void print_supported_drivers()
{
	std::string supported_drivers = "Supported drivers: ";
	int const num_drivers = SDL_GetNumRenderDrivers();
	for (int i = 0; i < num_drivers - 1; ++i)
	{
		SDL_RendererInfo info;
		SDL_GetRenderDriverInfo(i, &info);
		supported_drivers += info.name;
		supported_drivers += ", ";
	}

	SDL_RendererInfo info;
	SDL_GetRenderDriverInfo(num_drivers - 1, &info);
	supported_drivers += info.name;

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "%s", supported_drivers.c_str());
}

int main(int argc, char** argv)
{
	SDL_LogSetAllPriority(SDL_LOG_PRIORITY_DEBUG);

	if (int result = SDL_Init(SDL_INIT_VIDEO))
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not initialize SDL: %s", SDL_GetError());
		return result;
	}

	SDL_version linked;
	SDL_GetVersion(&linked);

	SDL_LogInfo(SDL_LOG_CATEGORY_APPLICATION, "Linked SDL %d.%d.%d", linked.major, linked.minor, linked.patch);

	print_supported_drivers();
	
	int result;
	try
	{
		omm::program mm3;
		result = mm3.run(argc, argv);
	}
	catch (std::exception& e)
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Program quit by exception: %s", e.what());
		result = -1;
	}
	
	SDL_Quit();

	return result;
}
