#include "program.h"

#include <SDL.h>
#include <SDL2/macro.h>

#include <string>

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
