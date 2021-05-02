#pragma once

#define OMM_SDL_FAILURE(str) \
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Unhandled error at %s:%d (%s): %s", __FILE__, __LINE__, str, SDL_GetError()); \
    std::abort() 
#define OMM_SDL_FAILURE_IF(expr) \
    if(expr) { \
        OMM_SDL_FAILURE(#expr); \
    } sizeof(0)
#define OMM_SDL_FAILURE_IF_MSG(expr, msg) \
    if(expr) { \
        OMM_SDL_FAILURE(msg); \
    } sizeof(0)
#define OMM_SDL_ENSURE(expr) \
    if(auto const error = expr; error < 0) { \
        OMM_SDL_FAILURE(#expr); \
    } sizeof(0)    