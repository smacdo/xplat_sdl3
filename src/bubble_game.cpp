#include "bubble_game.h"

#include <forge/content.h>
#include <forge/support/sdl_support.h>

#include <SDL3/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <random>

// TODO: Spawn bubbles in waves
// TODO: Spawn random counts of bubbles.
// TODO: Make game bubble speed independent of window dimensions.
// TODO: Scale bubbles to size of window.
// TODO: Draw a gradient water background.
// TODO: Play a bubble pop sound.
// TODO: Display the number of bubbles popped.
// TODO: Draw debug stats every N seconds (1 sec screen, 5 console)
//        - time per update() (average, min, max)
//        - time per render()
//        - number of update, render calls / second
//        - memory use

constexpr bool DEBUG_RENDER_ENTITY = false;
constexpr bool DEBUG_RENDER_POP = false;

constexpr int BUBBLE_COUNT_MAX = 25;
constexpr int BUBBLE_COUNT_MIN = 25;

constexpr float BUBBLE_PIXEL_WIDTH_AND_HEIGHT = 512.f;
constexpr float BUBBLE_MIN_FLOAT_SPEED = 90.f;
constexpr float BUBBLE_MAX_FLOAT_SPEED = 150.f;
constexpr float BUBBLE_MIN_X = 0.f;
constexpr float BUBBLE_MAX_X = 300.f;
constexpr float BUBBLE_MIN_WOBBLE_X = 0.05f; // amplitude
constexpr float BUBBLE_MAX_WOBBLE_X = 1.f;
constexpr float BUBBLE_MIN_WOBBLE_PERIOD = 0.2f;
constexpr float BUBBLE_MAX_WOBBLE_PERIOD = 2.0f;
constexpr float BUBBLE_MIN_WOBBLE_OFFSET = 0.0f;
constexpr float BUBBLE_MAX_WOBBLE_OFFSET = M_2_PI;
constexpr float ONE_OVER_SQRT_TWO = 0.7071067812;

constexpr std::array<float, 4> BUBBLE_SIZES = {48.0f, 64.0f, 72.0f, 128.0f};

BubbleGame::BubbleGame(
    unique_sdl_renderer_ptr renderer,
    unique_sdl_window_ptr window)
    : Game(std::move(renderer), std::move(window)),
      random_device_(),
      random_engine_(random_device_()) {}

SDL_AppResult BubbleGame::on_init() {
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
      static_cast<float>(pixel_width()) - max_bubble_size / 2.f);

  std::uniform_real_distribution<float> speed_distribution(
      BUBBLE_MIN_FLOAT_SPEED, BUBBLE_MAX_FLOAT_SPEED);

  std::uniform_real_distribution<float> wobble_x_distribution(
      BUBBLE_MIN_WOBBLE_X, BUBBLE_MAX_WOBBLE_X);

  std::uniform_real_distribution<float> wobble_p_distribution(
      BUBBLE_MIN_WOBBLE_PERIOD, BUBBLE_MAX_WOBBLE_PERIOD);

  std::uniform_real_distribution<float> wobble_offset_distribution(
      BUBBLE_MIN_WOBBLE_OFFSET, BUBBLE_MAX_WOBBLE_OFFSET);

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
      bubble.radius = bubble.size / 2.f * ONE_OVER_SQRT_TWO;
      bubble.speed = speed_distribution(random_engine_);
      bubble.wobble_x = wobble_x_distribution(random_engine_);
      bubble.wobble_period = wobble_p_distribution(random_engine_);
      bubble.wobble_offset = wobble_offset_distribution(random_engine_);

      spawn_count--;
    }
  }

  // Make the bubbles float upwards.
  for (auto& bubble : bubbles_) {
    if (!bubble.alive) {
      continue;
    }

    bubble.y += bubble.speed * delta_s;
    // bubble.x += sin(bubble.y / 25) * 2.5;
    bubble.x +=
        sin(bubble.wobble_offset + elapsed_time_s_ * bubble.wobble_period) *
        bubble.wobble_x;

    // sin(_controller.value * 2 * pi + bubble.y * 10) * 0.005
  }

  // Despawn bubbles when they float past the top.
  for (auto& bubble : bubbles_) {
    if (bubble.y >= pixel_height() + bubble.size) {
      bubble.alive = false;
    }
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult BubbleGame::on_render(float delta_s, float extrapolation) {
  const auto future_time_s =
      (elapsed_time_s_ * 1000.f + kMsPerUpdate * extrapolation) / 1000.f;

  // Draw a color that changes over time.
  SDL_SetRenderDrawColor(renderer_.get(), 25, 150, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer_.get());

  SDL_SetRenderDrawColor(renderer_.get(), 255, 0, 255, SDL_ALPHA_OPAQUE);

  // Draw bubbles on the screen.
  for (const auto& bubble : bubbles_) {
    draw_bubble(bubble.x, bubble.y, bubble.size);
  }

  // Handle debug drawing.
  // TODO: refactor.
  if (debug_draw_time_left_s > 0.0f) {
    SDL_RenderLine(renderer_.get(), debug_mx_, debug_my_, debug_bx_, debug_by_);

    SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 255);
    SDL_RenderPoint(renderer_.get(), debug_mx_, debug_my_);

    debug_draw_time_left_s -= delta_s;
  }

  // Done!
  SDL_RenderPresent(renderer_.get());
  return SDL_APP_SUCCESS;
}

SDL_AppResult BubbleGame::draw_bubble(float x, float y, float size) const {
  SDL_assert(bubble_texture_.get() != nullptr);
  const auto half_size = size / 2.f;

  const auto top = y + half_size;
  const auto left = x - half_size;
  const auto bottom = y - half_size;
  const auto right = x + half_size;

  SDL_FRect src_rect{
      0, 0, BUBBLE_PIXEL_WIDTH_AND_HEIGHT, BUBBLE_PIXEL_WIDTH_AND_HEIGHT};
  SDL_FRect dest_rect{left, pixel_height() - top, size, size};

  if (!SDL_RenderTexture(
          renderer_.get(), bubble_texture_.get(), &src_rect, &dest_rect)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Debug helpers:
  if (DEBUG_RENDER_ENTITY) {
    //  Show the rendered rectangle.
    SDL_SetRenderDrawColor(renderer_.get(), 255, 0, 255, SDL_ALPHA_OPAQUE);
    SDL_RenderRect(renderer_.get(), &dest_rect);

    // Show the sprite center.
    SDL_SetRenderDrawColor(renderer_.get(), 255, 255, 255, 255);
    SDL_RenderPoint(renderer_.get(), x, pixel_height() - y);
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult BubbleGame::on_mouse_click(int mouse_x, int mouse_y) {
  pop_bubble_at(mouse_x, pixel_height() - mouse_y);
  return SDL_APP_CONTINUE;
}

bool BubbleGame::pop_bubble_at(float x, float y) {
  // Check if any bubbles intersect the pop point. Pop the first bubble that
  // nmtches.
  for (auto& bubble : bubbles_) {
    const auto delta_x = x - bubble.x;
    const auto delta_y = y - bubble.y;
    const auto distance_squared = delta_x * delta_x + delta_y * delta_y;

    if (distance_squared < bubble.radius * bubble.radius) {
      SDL_Log(
          "pop bubble (%f, %f, %f) at (%f, %f) with dist = %f",
          bubble.x,
          bubble.y,
          bubble.radius,
          x,
          y,
          distance_squared);
      bubble.alive = false;

      if (DEBUG_RENDER_POP) {
        debug_draw_time_left_s = 10.0f;
        debug_mx_ = x;
        debug_my_ = pixel_height() - y;
        debug_bx_ = bubble.x;
        debug_by_ = pixel_height() - bubble.y;
      }

      return true;
    }
  }

  return false;
}

size_t BubbleGame::bubble_count() const {
  return std::count_if(
      bubbles_.begin(), bubbles_.end(), [](const auto& bubble) {
        return bubble.alive;
      });
}