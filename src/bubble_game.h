#pragma once

#include <forge/content.h>
#include <forge/game.h>

#include <SDL3/SDL.h>

#include <random>
#include <vector>

class BubbleGame : public Game {
public:
  BubbleGame(
      unique_sdl_renderer_ptr renderer,
      unique_sdl_window_ptr window);

protected:
  SDL_AppResult on_init() override;
  SDL_AppResult on_input(float delta_s) override;
  SDL_AppResult on_update(float delta_s) override;
  SDL_AppResult on_render(float delta_s, float extrapolation) override;
  SDL_AppResult on_mouse_click(int mouse_x, int mouse_y) override;

private:
  SDL_AppResult draw_bubble(float x, float y, float size) const;
  bool pop_bubble_at(float x, float y);
  size_t bubble_count() const;

private:
  std::random_device random_device_;
  std::default_random_engine random_engine_;

  struct Bubble {
    float x = 0.0;
    float y = 0.0;
    float size = 64.0;
    float radius = 0.0;
    float speed = 100.0;
    float wobble_x = 0.0f; // amplitude
    float wobble_period = 1.0f;
    float wobble_offset = 0.0f;
    bool alive = false;
  };

  std::vector<Bubble> bubbles_;
  unique_sdl_texture_ptr bubble_texture_;
  float elapsed_time_s_ = 0.0f;

  std::unique_ptr<SdlAudioBuffer> pop_audio_buffer_;

  // TODO: move to a debug helper.
  float debug_draw_time_left_s = 0.0f;
  float debug_mx_ = 0.0f;
  float debug_my_ = 0.0f;
  float debug_bx_ = 0.0f;
  float debug_by_ = 0.0f;
};