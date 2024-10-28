#include "bubble_game.h"

#include <forge/content.h>
#include <forge/support/sdl_support.h>

#include <SDL3/SDL.h>

#include <cmath>
#include <cstdio> // TODO: remove

BubbleGame::BubbleGame(
    unique_sdl_renderer_ptr renderer,
    unique_sdl_window_ptr window)
    : Game(std::move(renderer), std::move(window)) {}

SDL_AppResult BubbleGame::on_init() {
  bubble_ = load_texture(renderer_.get(), "content/bubble.png");

  if (bubble_ == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "failed to load bubble image");
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult BubbleGame::on_input(float delta_s) { return SDL_APP_SUCCESS; }

SDL_AppResult BubbleGame::on_update(float delta_s) {
  elapsed_time_s_ += delta_s;
  return SDL_APP_SUCCESS;
}

SDL_AppResult BubbleGame::on_render(float extrapolation) {
  const auto future_time_s =
      (elapsed_time_s_ * 1000.f + kMsPerUpdate * extrapolation) / 1000.f;

  // Draw a color that changes over time.
  SDL_SetRenderDrawColor(
      renderer_.get(),
      (std::sin(future_time_s) + 1) / 2.0 * 255,
      (std::sin(future_time_s / 2) + 1) / 2.0 * 255,
      (std::sin(future_time_s) * 2 + 1) / 2.0 * 255,
      SDL_ALPHA_OPAQUE);

  SDL_RenderClear(renderer_.get());

  // Draw a bubble image on the screen.
  SDL_assert(bubble_.get() != nullptr);

  SDL_FRect src_rect{0, 0, 512, 512};
  SDL_FRect dest_rect{0, 0, 256, 256};

  if (!SDL_RenderTexture(
          renderer_.get(), bubble_.get(), &src_rect, &dest_rect)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Done!
  SDL_RenderPresent(renderer_.get());
  return SDL_APP_SUCCESS;
}