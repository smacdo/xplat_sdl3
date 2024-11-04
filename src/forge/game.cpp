#include <forge/game.h>

#include <forge/support/sdl_support.h>

#include <stdio.h>  // TODO: remove after debugging
#include <unistd.h> // getcwd

Game::Game(unique_sdl_renderer_ptr renderer, unique_sdl_window_ptr window)
    : renderer_(std::move(renderer)),
      window_(std::move(window)) {}

Game::~Game() {}

SDL_AppResult Game::init() {
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

  // Show the main window.
  SDL_ShowWindow(window_.get());

  // Print some debug information about the app window.
  int width = 0, height = 0, bbwidth = 0, bbheight = 0;

  SDL_GetWindowSize(window_.get(), &width, &height);
  SDL_GetWindowSizeInPixels(window_.get(), &bbwidth, &bbheight);

  SDL_Log("Window size: %ix%i", width, height);
  SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);

  if (width != bbwidth) {
    SDL_Log("High DPI environment detected");
  }

  // Initialize the actual game.
  return on_init();
}

SDL_AppResult Game::handle_event(SDL_Event* event) {
  SDL_assert(event != nullptr);

  switch (event->type) {
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
      const auto new_width = event->window.data1;
      const auto new_height = event->window.data2;

      SDL_Log(
          "Game::handle_event SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, w = %d, h = "
          "%d",
          new_width,
          new_height);

      return on_render_resized(new_width, new_height);
    }
    case SDL_EVENT_QUIT:
      SDL_Log("Game::handle_event SDL_EVENT_QUIT, quit_requested => true");
      quit_requested_ = true;
      break;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult Game::iterate() {
  // TODO: Handle debugger or other excessively long pauses

  // Measure the amount of time that has elapsed.
  // Ref: https://gameprogrammingpatterns.com/game-loop.html
  const auto current_time_ms = SDL_GetTicks();
  const auto elapsed_time_ms =
      (previous_time_ms_ > 0 ? current_time_ms - previous_time_ms_ : 0);

  previous_time_ms_ = current_time_ms;
  lag_time_ms_ += elapsed_time_ms;

  const float delta_s = elapsed_time_ms / 1000.f;

  // Process input prior to updating the simulation or rendering.
  if (on_input(delta_s) == SDL_APP_FAILURE) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Game input failed");
    return SDL_APP_FAILURE;
  }

  // Advance the simulation by running as many fixed time steps as required to
  // get `lag_time_ms_` lower than amount of delta time between logic updates.
  // TODO: Detect when sim updates exceed the allowed delta time.
  while (lag_time_ms_ >= kMsPerUpdate) {
    if (on_update(kMsPerUpdate / 1000.f) == SDL_APP_FAILURE) {
      SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "Game iteration failed");
      return SDL_APP_FAILURE;
    }

    lag_time_ms_ -= kMsPerUpdate;
  }

  // Render the game.
  // TODO: Detect when the renderer exceeds the allowed delta time.
  on_render(lag_time_ms_ / static_cast<float>(kMsPerUpdate));

  // Check if the user wants to continue running the game or if its tiem to
  // quit.
  return quit_requested_ ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

SDL_AppResult Game::on_init() { return SDL_APP_CONTINUE; }

SDL_AppResult Game::on_input(float /*delta_s*/) { return SDL_APP_CONTINUE; }

SDL_AppResult Game::on_update(float /*delta_s*/) { return SDL_APP_CONTINUE; }

SDL_AppResult Game::on_render(float /*extrapolation*/) {
  SDL_SetRenderDrawColor(renderer_.get(), 1.0f, 1.0f, 0.0f, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_.get());
  SDL_RenderPresent(renderer_.get());

  return SDL_APP_CONTINUE;
}

SDL_AppResult Game::on_render_resized(int width, int height) {
  return SDL_APP_CONTINUE;
}
