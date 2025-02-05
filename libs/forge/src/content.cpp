#include "forge/audio_manager.h"

#include <forge/content.h>

#include <forge/support/sdl_support.h>
#include <forge/support/stb_support.h>

#include <SDL3/SDL.h>
#include <stb/stb_image.h>

#include <format>

// TODO: Support .ogg files.

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

  // Resample the audio file to match the game's expected audio spec.
  // TODO: Cache the temporary audio streams used for conversion between loads.
  if (audio_buffer->spec.format != DEFAULT_AUDIO_SPEC.format ||
      audio_buffer->spec.channels != DEFAULT_AUDIO_SPEC.channels ||
      audio_buffer->spec.freq != DEFAULT_AUDIO_SPEC.freq) {
    SDL_LogMessage(
        SDL_LOG_CATEGORY_APPLICATION,
        SDL_LOG_PRIORITY_DEBUG,
        "resampling audio buffer from format = %x, channels = %d, freq = %d "
        "to format = %x, channels = %d, freq = %d",
        audio_buffer->spec.format,
        audio_buffer->spec.channels,
        audio_buffer->spec.freq,
        DEFAULT_AUDIO_SPEC.format,
        DEFAULT_AUDIO_SPEC.channels,
        DEFAULT_AUDIO_SPEC.freq);

    // Create a SDL audio stream that takes input matching this audio buffer's
    // spec and outputs as the game's expected audio spec.
    std::unique_ptr<SDL_AudioStream, SdlAudioStreamDestroyer> conversion_stream{
        SDL_CreateAudioStream(&audio_buffer->spec, &DEFAULT_AUDIO_SPEC)};

    if (conversion_stream == nullptr) {
      SDL_LogError(
          SDL_LOG_CATEGORY_APPLICATION,
          "failed to create audio stream for converting audio file: %s",
          SDL_GetError());
      return nullptr;
    }

    // Put the entire audio buffer into the stream and then flush it to let SDL
    // know there are no more bytes.
    if (!SDL_PutAudioStreamData(
            conversion_stream.get(),
            audio_buffer->data,
            audio_buffer->size_in_bytes)) {
      SDL_LogError(
          FORGE_LOG_CATEGORY_AUDIO,
          "failed to put audio content into audio conversion stream: %s",
          SDL_GetError());
      return nullptr;
    }

    SDL_FlushAudioStream(conversion_stream.get());

    // How many bytes are in the newly converted audio buffer?
    const int converted_buffer_size_in_bytes =
        SDL_GetAudioStreamAvailable(conversion_stream.get());

    if (converted_buffer_size_in_bytes == -1) {
      SDL_LogError(
          FORGE_LOG_CATEGORY_AUDIO,
          "failed to query number of bytes in converted audio stream: %s",
          SDL_GetError());
      return nullptr;
    }

    // Re-allocate the audio buffer prior to reading the converted bytes out.
    SDL_free(audio_buffer->data);

    audio_buffer->spec.format = DEFAULT_AUDIO_SPEC.format;
    audio_buffer->size_in_bytes = converted_buffer_size_in_bytes;
    audio_buffer->data =
        static_cast<uint8_t*>(SDL_malloc(converted_buffer_size_in_bytes));

    // Read the audio buffer back out.
    const int converted_bytes_read = SDL_GetAudioStreamData(
        conversion_stream.get(),
        audio_buffer->data,
        audio_buffer->size_in_bytes);

    if (converted_bytes_read == -1) {
      SDL_LogError(
          FORGE_LOG_CATEGORY_AUDIO,
          "failed to read bytes from converted audio stream: %s",
          SDL_GetError());
      return nullptr;
    }
  }

  // Print debug information to the log identifying the audio file that was
  // loaded.
  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_DEBUG,
      "loaded wav audio file %.*s",
      static_cast<int>(filename.length()),
      filename.data());

  return audio_buffer;
}