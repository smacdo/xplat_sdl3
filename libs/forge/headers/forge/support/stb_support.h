#pragma once
#include <stb/stb_image.h>

/// Returns a `stbi_io_callbacks` struct that will use the SDL2 IO Stream API to
/// handle file i/o when loading images.
///
/// Use this function when calling any stb_image function ending in
/// `from_callbacks` such as `stbi_load_from_callbacks`.
///
/// NOTE: You **MUST** pass the pointer to the `SDL_IOStream*` as an argument
/// when calling functions like `stbi_load_from_callbacks` otherwise you will
/// get undefined behavior!!
///
/// # Example:
/// ```
/// SDL_IOStream* file_io_stream = SDL_IOFromFile(file_path, "rb")
///
/// const auto stbio = create_stbi_sdl2_io_callbacks();
/// int width = 0, height = 0;
///
/// unsigned char* image_bytes = stbi_load_from_callbacks(
///     &stbio, file_io_stream, &width, &height, nullptr, STBI_rgb_alpha);
/// ```
stbi_io_callbacks create_stbi_sdl2_io_callbacks();

/// Functor that frees the image bytes returned by a `stb_image` load function.
struct StbImageBytesDeleter {
  void operator()(unsigned char* image_bytes) const noexcept;
};