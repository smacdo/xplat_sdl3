#include "bubble_game.h"

#include <forge/content.h>
#include <forge/support/sdl_support.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

// TODO: Spawn new bubbles randomly.
// TODO: Spawn bubbles in waves
// TODO: Spawn random counts of bubbles.
// TODO: Update window dimensions when they change.
// TODO: Make game bubble speed independent of window dimensions.
// TODO: Scale bubbles to size of window.
// TODO: Pop bubbles.
// TODO: Draw a gradient water background.
// TODO: Play a bubble pop sound.
// TODO: Display the number of bubbles popped.

constexpr int BUBBLE_COUNT_MAX = 10;
constexpr int BUBBLE_COUNT_MIN = 5;

constexpr float BUBBLE_PIXEL_WIDTH_AND_HEIGHT = 512.f;
constexpr float BUBBLE_MIN_FLOAT_SPEED = 50.f;
constexpr float BUBBLE_MAX_FLOAT_SPEED = 75.f;
constexpr float BUBBLE_MIN_X = 0.f;
constexpr float BUBBLE_MAX_X = 300.f;
constexpr std::array<float, 4> BUBBLE_SIZES = {48.0f, 64.0f, 72.0f, 128.0f};

BubbleGame::BubbleGame(
    unique_sdl_renderer_ptr renderer,
    unique_sdl_window_ptr window)
    : Game(std::move(renderer), std::move(window)),
      random_device_(),
      random_engine_(random_device_()) {}

SDL_AppResult BubbleGame::on_init() {
  // Cache the window rendering size.
  if (!SDL_GetRenderOutputSize(
          renderer_.get(), &render_width_, &render_height_)) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION, "failed to query render output size");
    return SDL_APP_FAILURE;
  }

  SDL_LogInfo(
      SDL_LOG_CATEGORY_APPLICATION,
      "Render output is %dx%d",
      render_width_,
      render_height_);

  // Load bubble texture.
  bubble_texture_ = load_texture(renderer_.get(), "content/bubble.png");

  if (bubble_texture_ == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "failed to load bubble image");
    return SDL_APP_FAILURE;
  }

  // Instantiate the pool of bubbles.
  bubbles_.reserve(BUBBLE_COUNT_MAX);

  for (size_t i = 0; i < BUBBLE_COUNT_MAX; ++i) {
    bubbles_.push_back(Bubble{});
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult BubbleGame::on_input(float delta_s) { return SDL_APP_SUCCESS; }

SDL_AppResult BubbleGame::on_update(float delta_s) {
  elapsed_time_s_ += delta_s;

  // Randomizers for bubble properties when spawning.
  const auto max_bubble_size =
      *(std::max_element(BUBBLE_SIZES.begin(), BUBBLE_SIZES.end()));

  std::uniform_real_distribution<float> start_x_distribution(
      BUBBLE_MIN_X + max_bubble_size / 2.f,
      static_cast<float>(render_width_) - max_bubble_size / 2.f);

  std::uniform_real_distribution<float> speed_distribution(
      BUBBLE_MIN_FLOAT_SPEED, BUBBLE_MAX_FLOAT_SPEED);

  // Spawn bubbles when there are too few bubbles on the screen.
  const auto population = bubble_count();
  auto spawn_count = bubbles_.size() - population;

  for (auto& bubble : bubbles_) {
    // Stop searching for new spawn slots when the spawner has hit its target.
    if (spawn_count == 0) {
      break;
    }

    // Check if this bubble slot is dead, and if so then spawn a new bubble in.
    if (!bubble.alive) {
      bubble.alive = true;
      bubble.x = start_x_distribution(random_engine_);
      bubble.y = -bubble.size;
      bubble.speed = speed_distribution(random_engine_);

      spawn_count--;
    }
  }

  // Make the bubbles float upwards.
  // TODO: A nicer animation than simply moving up.
  for (auto& bubble : bubbles_) {
    if (!bubble.alive) {
      continue;
    }

    bubble.y += bubble.speed * delta_s;
  }

  // Despawn bubbles when they float past the top.
  for (auto& bubble : bubbles_) {
    if (bubble.y >= render_height_ + bubble.size) {
      bubble.alive = false;
    }
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
    draw_bubble(bubble.x, bubble.y, bubble.size);
  }

  // Done!
  SDL_RenderPresent(renderer_.get());
  return SDL_APP_SUCCESS;
}

SDL_AppResult BubbleGame::draw_bubble(float x, float y, float size) const {
  SDL_assert(bubble_texture_.get() != nullptr);

  SDL_FRect src_rect{
      0, 0, BUBBLE_PIXEL_WIDTH_AND_HEIGHT, BUBBLE_PIXEL_WIDTH_AND_HEIGHT};
  SDL_FRect dest_rect{x, render_height_ - y, size, size};

  if (!SDL_RenderTexture(
          renderer_.get(), bubble_texture_.get(), &src_rect, &dest_rect)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

size_t BubbleGame::bubble_count() const {
  return std::count_if(
      bubbles_.begin(), bubbles_.end(), [](const auto& bubble) {
        return bubble.alive;
      });
}