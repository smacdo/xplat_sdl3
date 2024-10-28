#include "bubble_game.h"

#include <forge/content.h>

#include <SDL3/SDL.h>

SDL_AppResult BubbleGame::on_init(SDL_Renderer* renderer, SDL_Window* window) {
  bubble_ = load_texture(renderer, "content/bubble.png");

  if (bubble_ == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "failed to load bubble image");
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult
    BubbleGame::on_iterate(SDL_Renderer* renderer, SDL_Window* window) {
  // Draw a color that changes over time.
  const auto time = SDL_GetTicks() / 1000.f;

  SDL_SetRenderDrawColor(
      renderer,
      (std::sin(time) + 1) / 2.0 * 255,
      (std::sin(time / 2) + 1) / 2.0 * 255,
      (std::sin(time) * 2 + 1) / 2.0 * 255,
      SDL_ALPHA_OPAQUE);

  SDL_RenderClear(renderer);

  // Draw a bubble image on the screen.
  SDL_assert(bubble_.get() != nullptr);

  SDL_FRect src_rect{0, 0, 512, 512};
  SDL_FRect dest_rect{0, 0, 256, 256};

  if (!SDL_RenderTexture(renderer, bubble_.get(), &src_rect, &dest_rect)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Done!
  SDL_RenderPresent(renderer);
  return SDL_APP_SUCCESS;
}