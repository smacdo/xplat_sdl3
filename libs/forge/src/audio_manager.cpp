//
// Created by smacd on 2/3/2025.
//

#include "../headers/forge/audio_manager.h"

AudioManager::AudioManager() {
#ifdef NDEBUG
  SDL_SetLogPriority(FORGE_LOG_CATEGORY_AUDIO, SDL_LOG_PRIORITY_INFO);
#else
  SDL_SetLogPriority(FORGE_LOG_CATEGORY_AUDIO, SDL_LOG_PRIORITY_DEBUG);
#endif
}

SDL_AppResult AudioManager::init() {
  // Open the machine's default audio device and begin playback.
  audio_device_id_ =
      SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);

  if (audio_device_id_ == -1) {
    SDL_LogError(
        FORGE_LOG_CATEGORY_AUDIO,
        "SDL_OpenAudioDevice error: %s",
        SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Query the default device's audio stream's spec.
  if (SDL_GetAudioDeviceFormat(
          audio_device_id_, &device_audio_spec_, nullptr)) {
    SDL_LogInfo(
        FORGE_LOG_CATEGORY_AUDIO,
        "opened default audio device with format = %.*s, channels = %d, freq = "
        "%d",
        static_cast<int>(audio_format_name(device_audio_spec_.format).length()),
        audio_format_name(device_audio_spec_.format).data(),
        device_audio_spec_.channels,
        device_audio_spec_.freq);
  } else {
    SDL_LogError(
        FORGE_LOG_CATEGORY_AUDIO,
        "SDL_GetAudioDeviceFormat error: %s",
        SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create an audio stream that will play samples to the audio device. It is
  // assumed that any audio buffer not matching this format will be converted at
  // content load time.
  default_audio_stream_.reset(
      SDL_CreateAudioStream(&DEFAULT_AUDIO_SPEC, &device_audio_spec_));

  if (default_audio_stream_) {
    SDL_LogInfo(
        FORGE_LOG_CATEGORY_AUDIO,
        "created default audio streams with format = %.*s, channels = %d, freq "
        "= "
        "%d",
        static_cast<int>(audio_format_name(DEFAULT_AUDIO_SPEC.format).length()),
        audio_format_name(DEFAULT_AUDIO_SPEC.format).data(),
        DEFAULT_AUDIO_SPEC.channels,
        DEFAULT_AUDIO_SPEC.freq);
  } else {
    SDL_LogError(
        FORGE_LOG_CATEGORY_AUDIO,
        "SDL_CreateAudioStream error: %s",
        SDL_GetError());
    return SDL_APP_FAILURE;
  }

  if (!SDL_BindAudioStream(audio_device_id_, default_audio_stream_.get())) {
    SDL_LogError(
        FORGE_LOG_CATEGORY_AUDIO,
        "SDL_BindAudioStream error: %s",
        SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Success!
  return SDL_APP_CONTINUE;
}

bool AudioManager::play_once(const SdlAudioBuffer* buffer) const {
  SDL_assert(buffer != nullptr);

  // Refuse to play samples with a different format than the game's default. All
  // audio buffers should be converted at content load time.
  if (buffer->spec.format != DEFAULT_AUDIO_SPEC.format ||
      buffer->spec.channels != DEFAULT_AUDIO_SPEC.channels ||
      buffer->spec.freq != DEFAULT_AUDIO_SPEC.freq) {

    SDL_LogError(
        FORGE_LOG_CATEGORY_AUDIO,
        "unexpected audio spec in call to play_once: format = %x, channels = "
        "%d, freq = %d",
        buffer->spec.format,
        buffer->spec.channels,
        buffer->spec.freq);

    return false;
  }

  // Shove the entire audio buffer into the default audio stream.
  // TODO: Support playing multiple sounds at once.
  if (!SDL_PutAudioStreamData(
          default_audio_stream_.get(), buffer->data, buffer->size_in_bytes)) {
    SDL_LogError(
        FORGE_LOG_CATEGORY_AUDIO,
        "AudioManager::playOnce SDL_PutAudioStreamData error: %s",
        SDL_GetError());
    return false;
  }

  return true;
}