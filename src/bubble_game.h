#pragma once

#include <forge/content.h>
#include <forge/game.h>

#include <SDL3/SDL.h>

class BubbleGame : public Game {
public:
  BubbleGame(unique_sdl_renderer_ptr renderer, unique_sdl_window_ptr window);

protected:
  SDL_AppResult on_init() override;
  SDL_AppResult on_input(float delta_s) override;
  SDL_AppResult on_update(float delta_s) override;
  SDL_AppResult on_render(float extrapolation) override;

private:
  unique_sdl_texture_ptr bubble_;
  float elapsed_time_s_ = 0.0f;
};