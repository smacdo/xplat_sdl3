#pragma once

#include <forge/content.h>
#include <forge/game.h>

#include <SDL3/SDL.h>

#include <random>
#include <vector>

class BubbleGame : public Game {
public:
  BubbleGame(unique_sdl_renderer_ptr renderer, unique_sdl_window_ptr window);

protected:
  SDL_AppResult on_init() override;
  SDL_AppResult on_input(float delta_s) override;
  SDL_AppResult on_update(float delta_s) override;
  SDL_AppResult on_render(float extrapolation) override;

private:
  SDL_AppResult draw_bubble(float x, float y, float size) const;
  size_t bubble_count() const;

private:
  std::random_device random_device_;
  std::default_random_engine random_engine_;

  struct Bubble {
    float x = 0.0;
    float y = 0.0;
    float size = 64.0;
    float speed = 100.0;
    bool alive = false;
  };

  std::vector<Bubble> bubbles_;
  unique_sdl_texture_ptr bubble_texture_;
  float elapsed_time_s_ = 0.0f;
  int render_width_ = 0;
  int render_height_ = 0;
};