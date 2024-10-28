#include <forge/content.h>
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
      unique_sdl_texture_ptr bubble)
      : window(window),
        renderer(renderer),
        bubble(std::move(bubble)) {}

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_AppResult app_quit = SDL_APP_CONTINUE;

  unique_sdl_texture_ptr bubble = nullptr;
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
  // TODO: Move this section to dedicated gameplay module.
  auto bubble = load_texture(renderer, "content/bubble.png");

  if (bubble == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "failed to load bubble image");
    return SDL_APP_FAILURE;
  }

  // Initialize application context state now that the application has been
  // succesfully initialized.
  auto app_state = reinterpret_cast<AppState**>(app_state_in);
  *app_state = new AppState{window, renderer, std::move(bubble)};

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

  SDL_assert(app_state->bubble.get() != nullptr);

  SDL_FRect src_rect{0, 0, 512, 512};
  SDL_FRect dest_rect{0, 0, 256, 256};

  if (!SDL_RenderTexture(
          app_state->renderer,
          app_state->bubble.get(),
          &src_rect,
          &dest_rect)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

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
