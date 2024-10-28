#pragma once

#include <forge/content.h>
#include <forge/game.h>

#include <SDL3/SDL.h>

class BubbleGame : public Game {
public:
  SDL_AppResult on_init(SDL_Renderer* renderer, SDL_Window* window) override;
  SDL_AppResult on_iterate(SDL_Renderer* renderer, SDL_Window* window) override;

private:
  unique_sdl_texture_ptr bubble_;
};