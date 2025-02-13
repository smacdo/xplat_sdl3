#include "forge/audio_manager.h"

#include <forge/content.h>

#include <forge/support/sdl_support.h>
#include <forge/support/stb_support.h>

#include <SDL3/SDL.h>
#include <stb/stb_image.h>
#include <stb/stb_vorbis.h>

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

std::vector<unsigned char> load_binary(const std::string_view filename) {
  const auto full_path = std::format("{}{}", SDL_GetBasePath(), filename);

  // Open file stream to the binary file.
  std::unique_ptr<SDL_IOStream, SdlIoCloser> file_io_stream{
      SDL_IOFromFile(full_path.c_str(), "rb")};

  if (file_io_stream == nullptr) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to open file io stream: %s",
        SDL_GetError());

    return {};
  }

  // Read the file size prior to allocating a buffer for the file contents.
  int64_t file_size_in_bytes = SDL_GetIOSize(file_io_stream.get());

  if (file_size_in_bytes == 0) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to query size of file io stream: %s",
        SDL_GetError());

    return {};
  }

  // Allocate a memory buffer for the file contents, read it in and then return
  // it to the caller (assuming no errors).
  std::vector<unsigned char> buffer(file_size_in_bytes);
  size_t bytes_read =
      SDL_ReadIO(file_io_stream.get(), buffer.data(), file_size_in_bytes);

  if (bytes_read == 0) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to read binary file io stream to byte buffer: %s",
        SDL_GetError());

    return {};
  } else if (bytes_read != file_size_in_bytes) {
    SDL_LogWarn(
        SDL_LOG_CATEGORY_APPLICATION,
        "expected file to read %d bytes but read %d bytes instead",
        file_size_in_bytes,
        bytes_read);
  }

  return buffer;
}

std::unique_ptr<SdlAudioBuffer> load_ogg(const std::string_view filename) {
  // Fully load the file as a binary blob.
  //
  // This can be optimized later to read only chunks of the file, decode, and
  // pushed into an SDL audio buffer as an optimization task.
  std::vector<unsigned char> ogg_bytes = load_binary(filename);

  if (ogg_bytes.empty()) {
    return nullptr;
  }

  // Decode the ogg file into an array of S16 samples.
  auto audio_buffer = std::make_unique<SdlAudioBuffer>();
  audio_buffer->spec.format = SDL_AUDIO_S16;

  const auto samples_read = stb_vorbis_decode_memory(
      ogg_bytes.data(),
      ogg_bytes.size(),
      &(audio_buffer->spec.channels),
      &(audio_buffer->spec.freq),
      reinterpret_cast<short**>(&(audio_buffer->data)));

  if (samples_read < 0) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "ogg audio file loading failed: %.*s",
        static_cast<int>(filename.length()),
        filename.data());
    return nullptr;
  }

  // Store the size of the decoded audio buffer in bytes rather than S16 sample
  // count.
  const size_t audio_buffer_size_in_bytes = samples_read * sizeof(short);
  SDL_assert(
      audio_buffer_size_in_bytes <= std::numeric_limits<uint32_t>::max());

  audio_buffer->size_in_bytes =
      static_cast<uint32_t>(audio_buffer_size_in_bytes);

  // Convert the decoded S16 ogg format into the game's target sound format
  // prior to returning the loaded buffer.
  if (samples_read == 0) {
    SDL_LogWarn(
        SDL_LOG_CATEGORY_APPLICATION,
        "no audio samples when loading ogg audio file: %.*s",
        static_cast<int>(filename.length()),
        filename.data());
  }

  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_DEBUG,
      "loaded ogg audio file %.*s",
      static_cast<int>(filename.length()),
      filename.data());

  return resample_if_needed(std::move(audio_buffer), DEFAULT_AUDIO_SPEC);
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

  // Print debug information to the log identifying the audio file that was
  // loaded.
  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_DEBUG,
      "loaded wav audio file %.*s",
      static_cast<int>(filename.length()),
      filename.data());

  return resample_if_needed(std::move(audio_buffer), DEFAULT_AUDIO_SPEC);
}