#pragma once

#include <forge/support/sdl_support.h>

#include <memory>
#include <string_view>

struct SDL_Renderer;

/// Loads an image from the game's content directory and returns it as a unique
/// pointer to a `SDL_Texture`.
///
/// # Example
/// ```
/// auto foo = load_texture(renderer, "content/foo.png");
/// ```
std::unique_ptr<SDL_Texture, SdlTextureCloser>
    load_texture(SDL_Renderer* renderer, const std::string_view filename);

/// Loads a wav audio file from the game's content directory.
std::unique_ptr<SdlAudioBuffer> load_wav(const std::string_view filename);