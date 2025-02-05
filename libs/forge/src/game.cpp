#include "forge/audio_manager.h"

#include <forge/game.h>

#include <forge/support/sdl_support.h>

#include <filesystem>

Game::Game(
    unique_sdl_renderer_ptr renderer,
    unique_sdl_window_ptr window)
    : renderer_(std::move(renderer)),
      window_(std::move(window)) {}

Game::~Game() = default;

SDL_AppResult Game::init() {
  // Print start up information to assist with troubleshooting.
  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_INFO,
      "app base path is %s",
      SDL_GetBasePath());

  // Initialize subsystems.
  audio_ = std::make_unique<AudioManager>();

  if (const auto audio_init_status = audio_->init();
      audio_init_status != SDL_APP_CONTINUE) {
    return audio_init_status;
  }

  // Show the main window.
  SDL_ShowWindow(window_.get());
  int width = 0, height = 0;

  SDL_GetWindowSize(window_.get(), &width, &height);
  SDL_GetWindowSizeInPixels(window_.get(), &pixel_width_, &pixel_height_);

  SDL_Log("Window size: %ix%i", width, height);
  SDL_Log("Back buffer size: %ix%i", pixel_width_, pixel_height_);

  if (width != pixel_width_) {
    SDL_Log(
        "High DPI environment detected, pixel density = %f",
        SDL_GetWindowPixelDensity(window_.get()));
  }

  // Initialize the actual game.
  return on_init();
}

SDL_AppResult Game::handle_event(const SDL_Event* event) {
  SDL_assert(event != nullptr);

  switch (event->type) { // NOLINT
    case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED: {
      pixel_width_ = event->window.data1;
      pixel_height_ = event->window.data2;

      SDL_Log(
          "Game::handle_event SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED, w = %d, h = "
          "%d",
          pixel_width_,
          pixel_height_);

      return on_render_resized(pixel_width_, pixel_height_);
    }
    case SDL_EVENT_FINGER_DOWN: {
      // Reject out of bounds touches.
      // Ref: https://wiki.libsdl.org/SDL3/SDL_TouchFingerEvent
      if (event->tfinger.x < 0.0 || event->tfinger.x > 1.0 ||
          event->tfinger.y < 0.0 || event->tfinger.y > 1.0) {
        break;
      }

      // Calculate touch location in window by converting from normalized [0, 1]
      // coordinates to render pixels.
      const auto touch_x = static_cast<int>(event->tfinger.x * pixel_width());
      const auto touch_y = static_cast<int>(event->tfinger.y * pixel_height());

      SDL_Log(
          "Game::handle_event SDL_EVENT_FINGER_DOWN, x = %d, y = %d",
          touch_x,
          touch_y);

      return on_touch_finger_down(touch_x, touch_y);
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      const auto pixel_density = SDL_GetWindowPixelDensity(window_.get());
      const auto mouse_x = static_cast<int>(event->button.x * pixel_density);
      const auto mouse_y = static_cast<int>(event->button.y * pixel_density);

      SDL_Log(
          "Game::handle_event SDL_EVENT_MOUSE_BUTTON_UP, x = %d, y = %d",
          mouse_x,
          mouse_y);

      return on_mouse_click(mouse_x, mouse_y);
    }
    case SDL_EVENT_QUIT:
      SDL_Log("Game::handle_event SDL_EVENT_QUIT, quit_requested => true");
      quit_requested_ = true;
      break;
    default:
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
  on_render(delta_s, lag_time_ms_ / static_cast<float>(kMsPerUpdate));

  // Check if the user wants to continue running the game or if it's time to
  // quit.
  return quit_requested_ ? SDL_APP_SUCCESS : SDL_APP_CONTINUE;
}

SDL_AppResult Game::on_init() { return SDL_APP_CONTINUE; }

SDL_AppResult Game::on_input(float /*delta_s*/) { return SDL_APP_CONTINUE; }

SDL_AppResult Game::on_update(float /*delta_s*/) { return SDL_APP_CONTINUE; }

SDL_AppResult Game::on_render(float /*delta_s*/, float /*extrapolation*/) {
  SDL_SetRenderDrawColor(renderer_.get(), 1.0f, 1.0f, 0.0f, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_.get());
  SDL_RenderPresent(renderer_.get());

  return SDL_APP_CONTINUE;
}

SDL_AppResult Game::on_render_resized(int width, int height) {
  return SDL_APP_CONTINUE;
}

SDL_AppResult Game::on_mouse_click(int mouse_x, int mouse_y) {
  return SDL_APP_CONTINUE;
}

SDL_AppResult Game::on_touch_finger_down(int touch_x, int touch_y) {
  return SDL_APP_CONTINUE;
}