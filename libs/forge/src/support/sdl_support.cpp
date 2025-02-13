#include <forge/support/sdl_support.h>

#include <SDL3/SDL.h>

SdlAudioBuffer::~SdlAudioBuffer() {
  SDL_free(data);

  size_in_bytes = 0;
  data = nullptr;
}

std::unique_ptr<SdlAudioBuffer> resample_if_needed(
    std::unique_ptr<SdlAudioBuffer> audio_buffer,
    const SDL_AudioSpec& target_spec) {
  // Only resample the audio buffer if it does not already match the game's
  // expected audio spec.
  if (audio_buffer->spec.format == target_spec.format &&
      audio_buffer->spec.channels == target_spec.channels &&
      audio_buffer->spec.freq == target_spec.freq) {
    return audio_buffer;
  }

  // The audio buffer does not match - resample it!
  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_DEBUG,
      "resampling audio buffer from format = %.*s, channels = %d, freq = %d "
      "to format = %.*s, channels = %d, freq = %d",
      static_cast<int>(audio_format_name(audio_buffer->spec.format).length()),
        audio_format_name(audio_buffer->spec.format).data(),
        audio_buffer->spec.channels,
        audio_buffer->spec.freq,
        static_cast<int>(audio_format_name(target_spec.format).length()),
        audio_format_name(target_spec.format).data(),
        target_spec.channels,
        target_spec.freq);

    // Create a SDL audio stream that takes input matching this audio buffer's
    // spec and outputs as the target audio spec.
    std::unique_ptr<SDL_AudioStream, SdlAudioStreamDestroyer> conversion_stream{
        SDL_CreateAudioStream(&audio_buffer->spec, &target_spec)};

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
          SDL_LOG_CATEGORY_APPLICATION,
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
          SDL_LOG_CATEGORY_APPLICATION,
          "failed to query number of bytes in converted audio stream: %s",
          SDL_GetError());
      return nullptr;
    }

    // Re-allocate the audio buffer prior to reading the converted bytes out.
    //
    // Both `SDL_LoadWav` and `stb_vorbis` use malloc and free so this is safe
    // to call without checking which library was responsible for allocating the
    // buffer.
    SDL_free(audio_buffer->data);

    audio_buffer->spec.format = target_spec.format;
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
          SDL_LOG_CATEGORY_APPLICATION,
          "failed to read bytes from converted audio stream: %s",
          SDL_GetError());
      return nullptr;
    }

  return audio_buffer;
}

void SdlAudioStreamDestroyer::operator()(
    SDL_AudioStream* stream) const noexcept {
  SDL_DestroyAudioStream(stream);
}

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

void SdlRendererDestroyer::operator()(SDL_Renderer* renderer) const noexcept {
  SDL_DestroyRenderer(renderer);
}

void SdlSurfaceCloser::operator()(SDL_Surface* surface) const noexcept {
  SDL_DestroySurface(surface);
}

void SdlTextureCloser::operator()(SDL_Texture* texture) const noexcept {
  SDL_DestroyTexture(texture);
}

void SdlWindowDestroyer::operator()(SDL_Window* window) const noexcept {
  SDL_DestroyWindow(window);
}