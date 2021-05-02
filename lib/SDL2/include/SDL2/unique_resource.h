#pragma once

#include <memory>

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;
struct SDL_Surface;

namespace omm::sdl 
{
    struct window_delete 
    {
        void operator()(SDL_Window* p) const noexcept;
    };

    struct renderer_delete
    {
        void operator()(SDL_Renderer* p) const noexcept;
    };

    struct texture_delete
    {
        void operator()(SDL_Texture* p) const noexcept;
    };

    struct surface_delete
    {
        void operator()(SDL_Surface* p) const noexcept;
    };

    using unique_window = std::unique_ptr<SDL_Window, window_delete>;
    using unique_renderer = std::unique_ptr<SDL_Renderer, renderer_delete>;
    using unique_texture = std::unique_ptr<SDL_Texture, texture_delete>;
    using unique_surface = std::unique_ptr<SDL_Surface, surface_delete>;
}