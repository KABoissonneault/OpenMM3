#pragma once

#include <iosfwd>
#include <span>
#include <cstdio>

#include <SDL2/unique_resource.h>

namespace omm::mm3
{
	[[nodiscard]] sdl::unique_surface raw_to_monochrome_rgba32(std::istream& i);
	[[nodiscard]] sdl::unique_surface raw_to_monochrome_rgba32(std::FILE* f);
}
