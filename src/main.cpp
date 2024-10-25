#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <cmath>

struct AppState {
  AppState(SDL_Window* window, SDL_Renderer* renderer)
      : window(window),
        renderer(renderer) {}

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_AppResult app_quit = SDL_APP_CONTINUE;
};

SDL_AppResult SDL_AppInit(void** app_state_in, int argc, char* argv[]) {
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

  // Initialize application context state now that the application has been
  // succesfully initialized.
  auto app_state = reinterpret_cast<AppState**>(app_state_in);
  *app_state = new AppState{window, renderer};

  SDL_Log("Application started successfully !");

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

  // Draw a color that changes over time.
  const auto time = SDL_GetTicks() / 1000.f;

  SDL_SetRenderDrawColor(
      app_state->renderer,
      (std::sin(time) + 1) / 2.0 * 255,
      (std::sin(time / 2) + 1) / 2.0 * 255,
      (std::sin(time) * 2 + 1) / 2.0 * 255,
      SDL_ALPHA_OPAQUE);

  SDL_RenderClear(app_state->renderer);
  SDL_RenderPresent(app_state->renderer);

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