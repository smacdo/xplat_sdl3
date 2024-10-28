#pragma once

#include <memory>

struct SDL_IOStream;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Window;

/// Functor that calls `SDL_CloseIO` on a `SDL_IOStream*`. Intended for use with
/// a smart pointer that automatically closes the stream when it goes out
/// scope.
///
/// ```
/// std::unique_ptr<SDL_IOStream, SdlIoCloser> io{
///   SDL_IOFromFile(path.c_str(),
///   "rb");
/// };
/// ```
struct SdlIoCloser {
  void operator()(SDL_IOStream* stream) const noexcept;
};

/// Functor that calls `SDL_DestroyRenderer` on a `SDL_Renderer`. Intended for
/// use with a smart pointer that automatically destroys the renderer when it
/// goes out of scope.
struct SdlRendererDestroyer {
  void operator()(SDL_Renderer* renderer) const noexcept;
};

using unique_sdl_renderer_ptr =
    std::unique_ptr<SDL_Renderer, SdlRendererDestroyer>;

/// Functor that calls `SDL_DestroySurface` on a `SDL_Surface*`. Intended for
/// use with a smart pointer that automatically closes the surface when it goes
/// out of scope.
///
/// ```
/// std::unique_ptr<SDL_Surface, SdlSurfaceCloser> surface{
///     SDL_CreateSurfaceFrom(...)
/// };
/// ```
struct SdlSurfaceCloser {
  void operator()(SDL_Surface* surface) const noexcept;
};

/// Functor that calls `SDL_DestroyTexture` on a `SDL_Texture*`. Intended for
/// use with a smart pointer that automatically closes the texture when it goes
/// out of scope.
///
/// ```
/// std::unique_ptr<SDL_Texture, SdlTextureCloser> surface{
///     SDL_CreateTextureFromSurface(...)
/// };
/// ```
struct SdlTextureCloser {
  void operator()(SDL_Texture* texture) const noexcept;
};

using unique_sdl_texture_ptr = std::unique_ptr<SDL_Texture, SdlTextureCloser>;

/// Functor that calls `SDL_DestroyWindow` on a `SDL_Window*`. Intended for use
/// with a smart pointer that automatically closes the window when it goes out
/// of scope
struct SdlWindowDestroyer {
  void operator()(SDL_Window* window) const noexcept;
};

using unique_sdl_window_ptr = std::unique_ptr<SDL_Window, SdlWindowDestroyer>;