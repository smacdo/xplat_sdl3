#include "bubble_game.h"

#include <forge/content.h>
#include <forge/support/sdl_support.h>

#include <SDL3/SDL.h>

#include <cmath>

// TODO: Reset bubbles when they hit the top of the screen.
// TODO: Spawn new bubbles randomly.
// TODO: Invert Y (world +Y should be up).
// TODO: Use window dimensions.
// TODO: Make game bubble speed independent of window dimensions.
// TODO: Scale bubbles to size of window.
// TODO: Bubbles should have different speeds.
// TODO: Pop bubbles.

constexpr float BUBBLE_PIXEL_WIDTH_AND_HEIGHT = 512.f;
constexpr float BUBBLE_FLOAT_SPEED = 100.f;

BubbleGame::BubbleGame(
    unique_sdl_renderer_ptr renderer,
    unique_sdl_window_ptr window)
    : Game(std::move(renderer), std::move(window)) {}

SDL_AppResult BubbleGame::on_init() {
  // Load bubble texture.
  bubble_texture_ = load_texture(renderer_.get(), "content/bubble.png");

  if (bubble_texture_ == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "failed to load bubble image");
    return SDL_APP_FAILURE;
  }

  // Create a random initial group of bubbles.
  constexpr int INITIAL_BUBBLE_COUNT = 5;

  for (int i = 0; i < INITIAL_BUBBLE_COUNT; ++i) {
    bubbles_.push_back(SDL_FRect{
        .x = i * 50.f,
        .y = 0.f,
        .w = 64.f,
        .h = 64.f,
    });
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult BubbleGame::on_input(float delta_s) { return SDL_APP_SUCCESS; }

SDL_AppResult BubbleGame::on_update(float delta_s) {
  elapsed_time_s_ += delta_s;

  // Make bubbles float upwards.
  // TODO: A nicer animation than simply moving up.
  for (auto& bubble : bubbles_) {
    bubble.y += BUBBLE_FLOAT_SPEED * delta_s;
  }

  return SDL_APP_SUCCESS;
}

SDL_AppResult BubbleGame::on_render(float extrapolation) {
  const auto future_time_s =
      (elapsed_time_s_ * 1000.f + kMsPerUpdate * extrapolation) / 1000.f;

  // Draw a color that changes over time.
  SDL_SetRenderDrawColor(renderer_.get(), 25, 150, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_.get());

  // Draw bubbles on the screen.
  for (const auto& bubble : bubbles_) {
    draw_bubble(bubble.x, bubble.y, bubble.w);
  }

  // Done!
  SDL_RenderPresent(renderer_.get());
  return SDL_APP_SUCCESS;
}

SDL_AppResult BubbleGame::draw_bubble(float x, float y, float size) const {
  SDL_assert(bubble_texture_.get() != nullptr);

  SDL_FRect src_rect{
      0, 0, BUBBLE_PIXEL_WIDTH_AND_HEIGHT, BUBBLE_PIXEL_WIDTH_AND_HEIGHT};
  SDL_FRect dest_rect{x, y, size, size};

  if (!SDL_RenderTexture(
          renderer_.get(), bubble_texture_.get(), &src_rect, &dest_rect)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}