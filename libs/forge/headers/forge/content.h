#pragma once

#include <forge/support/sdl_support.h>

#include <memory>
#include <string_view>
#include <vector>

struct SDL_Renderer;

/// Loads an image from the game's content directory and returns it as a unique
/// pointer to a `SDL_Texture`.
///
/// # Example
/// ```
/// auto foo = load_texture(renderer, "content/foo.png");
/// ```
std::unique_ptr<SDL_Texture, SdlTextureCloser>
    load_texture(SDL_Renderer* renderer, std::string_view filename);

/// Loads a file from the game's content directory and returns it as vector of
/// bytes.
std::vector<unsigned char> load_binary(std::string_view filename);

/// Loads a .ogg audio file from the game's content directory.
std::unique_ptr<SdlAudioBuffer> load_ogg(std::string_view filename);

/// Loads a .wav audio file from the game's content directory.
std::unique_ptr<SdlAudioBuffer> load_wav(std::string_view filename);