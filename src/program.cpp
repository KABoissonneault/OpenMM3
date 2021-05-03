#include "program.h"

#include "config.h"
#include "mm3/file.h"

#include <SDL2/macro.h>
#include <SDL_log.h>
#include <SDL_render.h>
#include <SDL_events.h>

#include <stdexcept>
#include <fstream>
#include <filesystem>
#include <optional>

#include <fmt/format.h>


namespace omm
{
	namespace
	{
		struct raw_image
		{
			sdl::unique_surface surface;
			sdl::unique_texture texture;
		};

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

		std::ifstream get_default_config(int argc, char** argv)
		{
			(void)argc;

			std::ifstream i;
			i.open("config.ini");

			if (i.is_open())
				return i;

			std::filesystem::path const current_executable(argv[0]);
			std::filesystem::path const adjacent_config = current_executable.parent_path() / "config.ini";

			i.open(adjacent_config.string());

			if (i.is_open())
				return i;

			throw std::runtime_error("Could not find 'config.ini'");
		}

		std::vector<std::filesystem::path> get_raw_file_paths(std::filesystem::path const& mm3_file_path)
		{
			std::filesystem::directory_iterator mm3_files(mm3_file_path);
			std::vector<std::filesystem::path> mm3_raw_files;

			for (std::filesystem::directory_entry const& entry : mm3_files)
			{
				if (entry.is_regular_file())
				{
					std::filesystem::path const& file = entry.path();
					if (file.extension() == ".raw")
						mm3_raw_files.push_back(file);
				}
			}

			return mm3_raw_files;
		}

		std::vector<raw_image> get_raw_images(std::filesystem::path const& mm3_file_path, SDL_Renderer* renderer)
		{
			std::vector<raw_image> raw_images;

			auto const raw_file_paths = get_raw_file_paths(mm3_file_path);

			for (auto const& raw_file_path : raw_file_paths)
			{
				std::FILE* raw_file = std::fopen(raw_file_path.string().c_str(), "rb");
				try
				{
					sdl::unique_surface surface = mm3::raw_to_monochrome_rgba32(raw_file);
					sdl::unique_texture texture(SDL_CreateTextureFromSurface(renderer, surface.get()));
					if (texture == nullptr)
						throw std::runtime_error(fmt::format("Could not create texture: %s", SDL_GetError()));
					raw_images.emplace_back(std::move(surface), std::move(texture));
				}
				catch (...)
				{
					std::fclose(raw_file);
					throw;
				}
				
				std::fclose(raw_file);
			}

			return raw_images;
		}

		void dump_strings(std::filesystem::path const& mm3_file_path, std::optional<std::filesystem::path> path_arg)
		{
			std::filesystem::path const output_path = path_arg.value_or("strings.csv");

			std::ofstream output_file(output_path);
			if (!output_file)
				throw std::runtime_error(fmt::format("Could not open '{}'", output_path.string()));

			output_file << "File;Index;Value\n";

			for (auto const& directory_entry : std::filesystem::directory_iterator(mm3_file_path))
			{
				if (!directory_entry.is_regular_file())
					continue;

				std::filesystem::path const& file_path = directory_entry.path();

				if (file_path.extension() != ".bin")
					continue;

				std::filesystem::path const filename = file_path.filename();
				int index = 0;

				std::ifstream string_file(file_path);
				if (!string_file)
				{
					SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Could not open string file '%s'", file_path.string().c_str());
					continue;
				}

				std::string line;
				while (std::getline(string_file, line, '\0'))
				{
					for (size_t endline_pos = line.find('\n'); endline_pos != std::string::npos; endline_pos = line.find('\n'))
						line.replace(endline_pos, 1, " / ");

					if (line.empty())
						continue;

					output_file << filename << ";" << index++ << ";\"" << line << "\"\n";
				}
			}
		}

		void handle_args(std::filesystem::path const& mm3_file_path, int argc, char** argv)
		{
			(void)argc;

			++argv; // Ignore executable

			while (char const* arg = *argv)
			{
				if (arg[0] != '-')
					throw std::runtime_error(fmt::format("Invalid program argument '{}'", arg));

				if (strcmp(arg, "--dump-strings") == 0)
				{
					char const* next_arg = argv[1];
					if (next_arg == nullptr || next_arg[0] == '-')
					{
						dump_strings(mm3_file_path, std::nullopt);
					}
					else
					{
						dump_strings(mm3_file_path, next_arg);
						++argv;
					}
				}

				++argv;
			}
		}
	}

	int program::initialize_window(int argc, char** argv)
	{
		(void)argc;
		(void)argv;

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

		std::ifstream config_file = get_default_config(argc, argv);
		std::filesystem::path mm3_file_path;
		config::visit_config(config_file, &mm3_file_path, [](void* obj, std::string_view section, std::string_view key, std::string_view value)
		{
			auto& mm3_files = *static_cast<std::filesystem::path*>(obj);
			if (section == "Core" && key == "FilesPath")
			{
				mm3_files = value;
			}
		});

		handle_args(mm3_file_path, argc, argv);

		auto const raw_images = get_raw_images(mm3_file_path, renderer.get());
		size_t image_index = 0;
		float scale = 4.f;

		while (true)
		{
			SDL_PumpEvents();

			SDL_Event e;
			while (SDL_PollEvent(&e))
			{
				switch (e.type)
				{
				case SDL_QUIT:
					return 0;

				case SDL_KEYDOWN:
					if (!e.key.repeat)
					{
						if (e.key.keysym.scancode == SDL_SCANCODE_UP)
							image_index = (image_index + 1) % raw_images.size();
						else if (e.key.keysym.scancode == SDL_SCANCODE_DOWN)
							image_index = (image_index + raw_images.size() - 1) % raw_images.size();
					}
				}
			}

			OMM_SDL_FAILURE_IF(SDL_SetRenderDrawColor(renderer.get(), 148, 155, 203, SDL_ALPHA_OPAQUE));
			OMM_SDL_FAILURE_IF(SDL_RenderClear(renderer.get()));

			if (raw_images.size() != 0)
			{
				raw_image const& im = raw_images[image_index];

				int output_w, output_h;
				OMM_SDL_FAILURE_IF(SDL_GetRendererOutputSize(renderer.get(), &output_w, &output_h));

				SDL_Rect dest;
				dest.w = im.surface->w * scale;
				dest.x = output_w / 2 - dest.w / 2;
				dest.h = im.surface->h * scale;
				dest.y = output_h / 2 - dest.h / 2;

				SDL_RenderCopy(renderer.get(), im.texture.get(), nullptr, &dest);
			}

			SDL_RenderPresent(renderer.get());
		}
	}
}