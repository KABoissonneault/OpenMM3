#include "mm3/file.h"

#include <iostream>

#include <fmt/format.h>

#include <SDL_surface.h>

namespace omm::mm3
{
	constexpr size_t raw_data_size = 64'000; // .raw files are always 64000 bytes

	namespace
	{
		void rgba_copy(std::span<std::byte> dst, std::span<std::byte const> src)
		{
			for (size_t i = 0; i < src.size(); ++i)
			{
				size_t const pixel_index = i * 3;
				dst[pixel_index + 0] = dst[pixel_index + 1] = dst[pixel_index + 2] = static_cast<std::byte>(src[i]);
			}
		}
	}

	sdl::unique_surface raw_to_monochrome_rgba32(std::istream& is)
	{
		sdl::unique_surface surface(SDL_CreateRGBSurfaceWithFormat(0, 320, 200, 24, SDL_PIXELFORMAT_RGB24));
		if (surface == nullptr)
			throw std::runtime_error(fmt::format("Could not create surface: {}", SDL_GetError()));

		std::span<std::byte, raw_data_size * 3> pixels(static_cast<std::byte*>(surface->pixels), raw_data_size * 3);

		size_t read = 0;
		do
		{
			char raw_data[4096];
			std::streamsize const read_size = std::min(std::size(raw_data), raw_data_size - read);
			if (!(is.get(raw_data, read_size)))
				throw std::runtime_error("Raw data didn't have 64,000 bytes");

			auto const dst = pixels.subspan(read * 3, read_size * 3);
			auto const src = std::span<std::byte const>(reinterpret_cast<std::byte*>(raw_data), read_size);
			rgba_copy(dst, src);

			read += read_size;
		} while (read < 64'000);

		return surface;
	}

	sdl::unique_surface raw_to_monochrome_rgba32(std::FILE* f)
	{
		sdl::unique_surface surface(SDL_CreateRGBSurfaceWithFormat(0, 320, 200, 24, SDL_PIXELFORMAT_RGB24));
		if (surface == nullptr)
			throw std::runtime_error(fmt::format("Could not create surface: {}", SDL_GetError()));

		std::span<std::byte, raw_data_size * 3> pixels(static_cast<std::byte*>(surface->pixels), raw_data_size * 3);

		size_t read = 0;
		do
		{
			char raw_data[4096];
			size_t const read_size = std::min(std::size(raw_data), raw_data_size - read);
			size_t const actually_read = std::fread(raw_data, 1, read_size, f);
			if(actually_read != read_size)
				throw std::runtime_error("Raw data didn't have 64,000 bytes");

			auto const dst = pixels.subspan(read * 3, read_size * 3);
			auto const src = std::span<std::byte const>(reinterpret_cast<std::byte*>(raw_data), read_size);
			rgba_copy(dst, src);

			read += read_size;
		} while (read < 64'000);

		return surface;
	}
}