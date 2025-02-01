#include <forge/support/stb_support.h>

#include <SDL3/SDL.h>
#include <stb/stb_image.h>

stbi_io_callbacks create_stbi_sdl2_io_callbacks() {
  return stbi_io_callbacks{
      .read = [](void* user, char* data, int size) -> int {
        SDL_assert(user != nullptr);
        auto stream = static_cast<SDL_IOStream*>(user);
        const auto bytes_read = SDL_ReadIO(stream, data, size);
        return static_cast<int>(bytes_read);
      },
      .skip = [](void* user, int n) -> void {
        SDL_assert(user != nullptr);
        auto stream = static_cast<SDL_IOStream*>(user);
        SDL_SeekIO(stream, n, SDL_IO_SEEK_CUR);
      },
      .eof = [](void* user) -> int {
        SDL_assert(user != nullptr);
        auto stream = static_cast<SDL_IOStream*>(user);
        return SDL_GetIOStatus(stream) == SDL_IO_STATUS_EOF ? 1 : 0;
      }};
}

void StbImageBytesDeleter::operator()(
    unsigned char* image_bytes) const noexcept {
  stbi_image_free(image_bytes);
}