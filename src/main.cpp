#include "bubble_game.h"

#include <forge/game.h>
#include <forge/support/sdl_support.h>
#include <forge/support/stb_support.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory>

using GameClass = BubbleGame;

struct AppState {
  AppState(std::unique_ptr<Game> game) : game(std::move(game)) {}
  std::unique_ptr<Game> game;
};

SDL_AppResult SDL_AppInit(void** app_state_in, int argc, char* argv[]) {
  // Make application log entries more visible in debug mode.
  // TODO: Make configurable at start up and runtime.
#ifdef NDEBUG
  SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
#else
  SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
#endif

  // Initialize SDL3 systems.
  if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create the SDL main window and renderer.
  // TODO: Make the window name customizable.
  std::unique_ptr<SDL_Window, SdlWindowDestroyer> window{
      SDL_CreateWindow("xplat_sdl3", 352, 430, SDL_WINDOW_RESIZABLE)};

  if (!window) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  std::unique_ptr<SDL_Renderer, SdlRendererDestroyer> renderer{
      SDL_CreateRenderer(window.get(), nullptr)};

  if (!renderer) {
    SDL_LogError(
        SDL_LOG_CATEGORY_CUSTOM,
        "SDL_CreateRenderer error: %s",
        SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create an audio stream to play sounds on the user's default audio device.
  constexpr SDL_AudioSpec audio_spec{
      .format = SDL_AUDIO_S16,
      .channels = 2,
      .freq = 44100,
  };

  std::unique_ptr<SDL_AudioStream, SdlAudioStreamDestroyer> device_audio_stream{
      SDL_OpenAudioDeviceStream(
          SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &audio_spec, nullptr, nullptr)};

  if (device_audio_stream) {
    SDL_ResumeAudioStreamDevice(device_audio_stream.get());
  } else {
    SDL_LogError(
        SDL_LOG_CATEGORY_CUSTOM,
        "SDL_OpenAudioDeviceStream error: %s",
        SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Initialize the game.
  auto game = std::make_unique<GameClass>(
      std::move(renderer), std::move(device_audio_stream), std::move(window));

  if (game->init() == SDL_APP_FAILURE) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Game failed to initialize");
    return SDL_APP_FAILURE;
  } else {
    SDL_Log("Game has been initialized");
  }

  // Store the game instance in the application context that is passed to all
  // the SDL application callbacks.
  auto app_state = reinterpret_cast<AppState**>(app_state_in);
  *app_state = new AppState{std::move(game)};

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(
    void* app_state_untyped,
    SDL_Event* event) { // NOLINT(readability-non-const-parameter)
  auto* app_state = static_cast<AppState*>(app_state_untyped);
  return app_state->game->handle_event(event);
}

SDL_AppResult SDL_AppIterate(void* app_state_untyped) {
  auto* app_state = static_cast<AppState*>(app_state_untyped);
  return app_state->game->iterate();
}

void SDL_AppQuit(void* app_state_untyped, SDL_AppResult result) {
  auto* app_state = static_cast<AppState*>(app_state_untyped);
  delete app_state;

  SDL_Log("Application quit successfully");
}