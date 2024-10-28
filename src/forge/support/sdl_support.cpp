#include <forge/support/sdl_support.h>

#include <SDL3/SDL.h>

void SdlIoCloser::operator()(SDL_IOStream* stream) const noexcept {
  if (!SDL_CloseIO(stream)) {
    // TODO: Print the name of the file after reverse engineering it from the
    //       associated file handle stored in the stream properties.
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to close file io stream (SDL Error: %s)",
        SDL_GetError());
  }
}

void SdlSurfaceCloser::operator()(SDL_Surface* surface) const noexcept {
  SDL_DestroySurface(surface);
}
