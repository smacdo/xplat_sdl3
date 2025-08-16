#pragma once

#include <SDL3/SDL.h>

class DebugOverlay {
public:
  void draw_line(
      float x1,
      float y1,
      float x2,
      float f2,
      float time_in_ms,
      uint8_t r,
      uint8_t g,
      uint8_t b);

  SDL_AppResult
      on_render(SDL_Renderer* renderer, SDL_Window* window, float delta_s);

private:
};