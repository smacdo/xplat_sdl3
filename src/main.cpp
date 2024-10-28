#include "bubble_game.h"

#include <forge/game.h>
#include <forge/support/sdl_support.h>
#include <forge/support/stb_support.h>

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

#include <memory>

#include <cmath>
#include <unistd.h> // getcwd

struct AppState {
  AppState(
      SDL_Window* window,
      SDL_Renderer* renderer,
      std::unique_ptr<Game> game)
      : window(window),
        renderer(renderer),
        game(std::move(game)) {}

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_AppResult app_quit = SDL_APP_CONTINUE;

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

  // Print start up information to assist with troubleshooting.
  char cwd[512];
  getcwd(cwd, sizeof(cwd));

  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_INFO,
      "app base path is %s",
      SDL_GetBasePath());
  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_INFO,
      "app working directory is %s",
      cwd);

  // Initialize SDL3 systems.
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create the SDL main window and renderer.
  SDL_Window* window =
      SDL_CreateWindow("Window", 352, 430, SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
  if (!renderer) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_ShowWindow(window);

  // Print some debug information about the app window.
  int width, height, bbwidth, bbheight;
  SDL_GetWindowSize(window, &width, &height);
  SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
  SDL_Log("Window size: %ix%i", width, height);
  SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);

  if (width != bbwidth) {
    SDL_Log("High DPI environment detected");
  }

  // Load game resources.
  auto game = std::make_unique<BubbleGame>();
  const auto result = game->on_init(renderer, window);

  if (result == SDL_APP_FAILURE) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Game failed to initialize");
    return SDL_APP_FAILURE;
  }

  SDL_Log("Game has been initialized");

  // Initialize application context state now that the application has been
  // succesfully initialized.
  auto app_state = reinterpret_cast<AppState**>(app_state_in);
  *app_state = new AppState{window, renderer, std::move(game)};

  SDL_Log("Application started successfully!");

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state_untyped, SDL_Event* event) {
  auto* app_state = reinterpret_cast<AppState*>(app_state_untyped);

  if (event->type == SDL_EVENT_QUIT) {
    app_state->app_quit = SDL_APP_SUCCESS;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* app_state_untyped) {
  auto* app_state = reinterpret_cast<AppState*>(app_state_untyped);
  const auto result =
      app_state->game->on_iterate(app_state->renderer, app_state->window);

  if (result == SDL_APP_FAILURE) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Game iteration failed");
    return SDL_APP_FAILURE;
  }

  return app_state->app_quit;
}

void SDL_AppQuit(void* app_state_untyped, SDL_AppResult result) {
  auto* app_state = reinterpret_cast<AppState*>(app_state_untyped);

  if (app_state != nullptr) {
    SDL_DestroyRenderer(app_state->renderer);
    SDL_DestroyWindow(app_state->window);
    delete app_state;
  }

  SDL_Log("Application quit successfully!");
}
