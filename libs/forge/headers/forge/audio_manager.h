#pragma once

#include <forge/support/sdl_support.h>

#include <SDL3/SDL.h>

constexpr auto FORGE_LOG_CATEGORY_AUDIO = SDL_LOG_CATEGORY_CUSTOM + 1;

constexpr SDL_AudioSpec DEFAULT_AUDIO_SPEC{
    .format = SDL_AUDIO_F32LE,
    .channels = 2,
    .freq = 44100,
};

class AudioManager {
public:
  AudioManager();
  SDL_AppResult init();
  bool play_once(const SdlAudioBuffer* buffer) const;

private:
  int audio_device_id_{-1};
  SDL_AudioSpec device_audio_spec_ = {};

  std::unique_ptr<SDL_AudioStream, SdlAudioStreamDestroyer>
      default_audio_stream_;
};