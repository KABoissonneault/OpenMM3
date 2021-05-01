#pragma once

#include <memory>

struct SDL_Window;
struct SDL_Renderer;

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

    using unique_window = std::unique_ptr<SDL_Window, window_delete>;
    using unique_renderer = std::unique_ptr<SDL_Renderer, renderer_delete>;
}