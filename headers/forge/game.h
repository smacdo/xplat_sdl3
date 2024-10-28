#pragma once

#include <SDL3/SDL.h>

class Game {
public:
  virtual ~Game();

  virtual SDL_AppResult on_init(SDL_Renderer* renderer, SDL_Window* window) = 0;
  virtual SDL_AppResult
      on_iterate(SDL_Renderer* renderer, SDL_Window* window) = 0;
};