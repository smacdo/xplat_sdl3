#pragma once

struct SDL_IOStream;
struct SDL_Surface;

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

/// Functor that calls `SDL_DestroySurface` on a `SDL_Surface*`. Intended for
/// use with a smart pointer that automatically closes the surface when it goes
/// out scope.
///
/// ```
/// std::unique_ptr<SDL_Surface, SdlSurfaceCloser> surface{
///     SDL_CreateSurfaceFrom(...)
/// };
/// ```
struct SdlSurfaceCloser {
  void operator()(SDL_Surface* surface) const noexcept;
};
