#include <forge/content.h>

#include <forge/support/sdl_support.h>
#include <forge/support/stb_support.h>

#include <SDL3/SDL.h>
#include <stb/stb_image.h>

#include <format>

std::unique_ptr<SDL_Texture, SdlTextureCloser>
    load_texture(SDL_Renderer* renderer, const std::string_view filename) {
  SDL_assert(renderer != nullptr);

  // Create the final file path relative to the game's resource directory.
  const auto full_path = std::format("{}{}", SDL_GetBasePath(), filename);

  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_INFO,
      "loading texture %.*s from path %s",
      static_cast<int>(filename.length()),
      filename.data(),
      full_path.c_str());

  // Read the requested file by wrapping stb_image's io callbacks with SDL's
  // IO streams API.
  std::unique_ptr<SDL_IOStream, SdlIoCloser> file_io_stream{
      SDL_IOFromFile(full_path.c_str(), "rb")};

  if (file_io_stream == nullptr) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to open file io stream: %s",
        SDL_GetError());

    return nullptr;
  }

  // Load image from disk into raw RGBA bytes using stb_image.
  const auto stbio = create_stbi_sdl2_io_callbacks();
  int width = 0, height = 0;

  std::unique_ptr<unsigned char, StbImageBytesDeleter> image_bytes{
      stbi_load_from_callbacks(
          &stbio,
          file_io_stream.get(),
          &width,
          &height,
          nullptr,
          STBI_rgb_alpha)};

  if (image_bytes == nullptr) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to load texture: %s",
        stbi_failure_reason());
    return nullptr;
  }

  // Create a new SDL texture with the same size as the loaded image, and then
  // blit the pixel bytes into the newly created texture.
  constexpr int RGBA_BYTES_PER_PIXEL = 4; // RGBA

  std::unique_ptr<SDL_Surface, SdlSurfaceCloser> surface{SDL_CreateSurfaceFrom(
      width,
      height,
      SDL_PIXELFORMAT_ARGB8888,
      image_bytes.get(),
      width * RGBA_BYTES_PER_PIXEL)};

  std::unique_ptr<SDL_Texture, SdlTextureCloser> texture{
      SDL_CreateTextureFromSurface(renderer, surface.get())};

  if (texture == nullptr) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to create sdl texture when loading texture: %s",
        SDL_GetError());

    return nullptr;
  }

  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_DEBUG,
      "loaded texture width = %d, height = %d, file = %.*s",
      width,
      height,
      static_cast<int>(filename.length()),
      filename.data());

  return texture;
}

std::unique_ptr<SdlAudioBuffer> load_wav(const std::string_view filename) {
  // Create the final file path relative to the game's resource directory.
  const auto full_path = std::format("{}{}", SDL_GetBasePath(), filename);

  // Load the wav file using SDL3.
  auto audio_buffer = std::make_unique<SdlAudioBuffer>();

  if (!SDL_LoadWAV(
          full_path.c_str(),
          &audio_buffer->spec,
          &audio_buffer->data,
          &audio_buffer->size_in_bytes)) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to create sdl audio buffer when loading wav file: %s",
        SDL_GetError());

    return nullptr;
  }

  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_DEBUG,
      "loaded audio buffer format = %x, channels = %d, freq = %d, file = %.*s",
      audio_buffer->spec.format,
      audio_buffer->spec.channels,
      audio_buffer->spec.freq,
      static_cast<int>(filename.length()),
      filename.data());

  return audio_buffer;
}