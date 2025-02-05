#pragma once

#include <SDL3/SDL_audio.h>

#include <memory>
#include <string_view>

struct SDL_AudioStream;
struct SDL_IOStream;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;
struct SDL_Window;

/// Returns a human-readable representation of an audio spec format value.
constexpr std::string_view audio_format_name(const SDL_AudioFormat format) {
  switch (format) {
    case SDL_AUDIO_U8:
      return "SDL_AUDIO_U8";
    case SDL_AUDIO_S8:
      return "SDL_AUDIO_S8";
    case SDL_AUDIO_S16LE:
      return "SDL_AUDIO_S16LE";
    case SDL_AUDIO_S16BE:
      return "SDL_AUDIO_S16BE";
    case SDL_AUDIO_S32LE:
      return "SDL_AUDIO_S32LE";
    case SDL_AUDIO_S32BE:
      return "SDL_AUDIO_S32BE";
    case SDL_AUDIO_F32LE:
      return "SDL_AUDIO_F32LE";
    case SDL_AUDIO_F32BE:
      return "SDL_AUDIO_F32BE";
    case SDL_AUDIO_UNKNOWN:
    default:
      return "SDL_AUDIO_UNKNOWN";
  }
}

/// Helper struct that represents an audio buffer allocated by SDL.
struct SdlAudioBuffer {
  uint32_t size_in_bytes = 0;
  uint8_t* data = nullptr;
  SDL_AudioSpec spec = {};

  ~SdlAudioBuffer();
};

/// Functor that calls `SDL_CloseIO` on a `SDL_AudioStream*`. Intended for use
/// with a smart pointer that automatically closes the stream when it goes out
/// scope.
struct SdlAudioStreamDestroyer {
  void operator()(SDL_AudioStream* stream) const noexcept;
};

using unique_sdl_audio_stream_ptr =
    std::unique_ptr<SDL_AudioStream, SdlAudioStreamDestroyer>;

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