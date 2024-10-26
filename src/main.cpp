#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <stb/stb_image.h>

#include <format>

#include <cmath>
#include <unistd.h> // getcwd

struct AppState {
  AppState(SDL_Window* window, SDL_Renderer* renderer, SDL_Texture* bubble)
      : window(window),
        renderer(renderer),
        bubble(bubble) {}

  SDL_Window* window = nullptr;
  SDL_Renderer* renderer = nullptr;
  SDL_AppResult app_quit = SDL_APP_CONTINUE;

  SDL_Texture* bubble = nullptr;
};

// TODO: wrap image_bytes with smart pointer to auto-delete.
// TODO: rewrite stbi_load to use sdl file loading.
SDL_Texture*
    load_texture(SDL_Renderer* renderer, const std::string_view filename) {
  SDL_assert(renderer != nullptr);

  // Create the final file path relative to the game's resource directory.
  const auto full_path = std::format("{}{}", SDL_GetBasePath(), filename);

  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_INFO,
      "loading texture %.*s from path %s",
      static_cast<int>(filename.length()),
      filename.data(),
      full_path.c_str());

  // Read the requested file by wrapping stb_image's io callbacks with SDL's
  // IO streams API.
  auto stbio = stbi_io_callbacks{
      .read = [](void* user, char* data, int size) -> int {
        SDL_assert(user != nullptr);
        auto stream = reinterpret_cast<SDL_IOStream*>(user);
        const auto bytes_read = SDL_ReadIO(stream, data, size);
        return static_cast<int>(bytes_read);
      },
      .skip = [](void* user, int n) -> void {
        SDL_assert(user != nullptr);
        auto stream = reinterpret_cast<SDL_IOStream*>(user);
        SDL_SeekIO(stream, n, SDL_IO_SEEK_CUR);
      },
      .eof = [](void* user) -> int {
        SDL_assert(user != nullptr);
        auto stream = reinterpret_cast<SDL_IOStream*>(user);
        return SDL_GetIOStatus(stream) == SDL_IO_STATUS_EOF ? 1 : 0;
      }};

  SDL_IOStream* file_io_stream = SDL_IOFromFile(full_path.c_str(), "rb");

  if (file_io_stream == nullptr) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to open file io stream: %s",
        SDL_GetError());

    return nullptr;
  }

  // Load image from disk into raw RGBA bytes using stb_image.
  int width = 0, height = 0, components = 0;

  unsigned char* image_bytes = stbi_load_from_callbacks(
      &stbio, file_io_stream, &width, &height, nullptr, STBI_rgb_alpha);

  if (image_bytes == nullptr) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to load texture: %s",
        stbi_failure_reason());
    SDL_CloseIO(file_io_stream); // TODO: warn if this returns false.
    return nullptr;
  }

  // Create a new SDL texture with the same size as the loaded image, and then
  // blit the pixel bytes into the newly created texture.
  constexpr int RGBA_BYTES_PER_PIXEL = 4; // RGBA

  SDL_Surface* surface = SDL_CreateSurfaceFrom(
      width,
      height,
      SDL_PIXELFORMAT_ARGB8888,
      image_bytes,
      width * RGBA_BYTES_PER_PIXEL);

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);

  if (texture == nullptr) {
    SDL_LogError(
        SDL_LOG_CATEGORY_APPLICATION,
        "failed to create sdl texture when loading texture: %s",
        SDL_GetError());

    stbi_image_free(image_bytes);
    SDL_CloseIO(file_io_stream); // TODO: warn if this returns false.

    return nullptr;
  }

  SDL_DestroySurface(surface);

  stbi_image_free(image_bytes);

  SDL_CloseIO(file_io_stream); // TODO: warn if this returns false.

  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_DEBUG,
      "loaded texture width = %d, height = %d, file = %.*s",
      width,
      height,
      static_cast<int>(filename.length()),
      filename.data());

  return texture;
}

SDL_AppResult SDL_AppInit(void** app_state_in, int argc, char* argv[]) {
  // Make application log entries more visible in debug mode.
  // TODO: Make configurable at start up and runtime.
#ifdef NDEBUG
  SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_INFO);
#else
  SDL_SetLogPriority(SDL_LOG_CATEGORY_APPLICATION, SDL_LOG_PRIORITY_DEBUG);
#endif

  // Print start up information to assist with troubleshooting.
  char cwd[512];
  getcwd(cwd, sizeof(cwd));

  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_INFO,
      "app base path is %s",
      SDL_GetBasePath());
  SDL_LogMessage(
      SDL_LOG_CATEGORY_APPLICATION,
      SDL_LOG_PRIORITY_INFO,
      "app working directory is %s",
      cwd);

  // Initialize SDL3 systems.
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  // Create the SDL main window and renderer.
  SDL_Window* window =
      SDL_CreateWindow("Window", 352, 430, SDL_WINDOW_RESIZABLE);
  if (!window) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_Renderer* renderer = SDL_CreateRenderer(window, NULL);
  if (!renderer) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_ShowWindow(window);

  // Print some debug information about the app window.
  int width, height, bbwidth, bbheight;
  SDL_GetWindowSize(window, &width, &height);
  SDL_GetWindowSizeInPixels(window, &bbwidth, &bbheight);
  SDL_Log("Window size: %ix%i", width, height);
  SDL_Log("Backbuffer size: %ix%i", bbwidth, bbheight);

  if (width != bbwidth) {
    SDL_Log("High DPI environment detected");
  }

  // Load game resources.
  // TODO: Move this section to dedicated gameplay module.
  SDL_Texture* bubble = load_texture(renderer, "content/bubble.png");

  if (bubble == nullptr) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "failed to load bubble image");
    return SDL_APP_FAILURE;
  }

  // Initialize application context state now that the application has been
  // succesfully initialized.
  auto app_state = reinterpret_cast<AppState**>(app_state_in);
  *app_state = new AppState{window, renderer, bubble};

  SDL_Log("Application started successfully !");

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* app_state_untyped, SDL_Event* event) {
  auto* app_state = reinterpret_cast<AppState*>(app_state_untyped);

  if (event->type == SDL_EVENT_QUIT) {
    app_state->app_quit = SDL_APP_SUCCESS;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppIterate(void* app_state_untyped) {
  auto* app_state = reinterpret_cast<AppState*>(app_state_untyped);

  // Draw a color that changes over time.
  const auto time = SDL_GetTicks() / 1000.f;

  SDL_SetRenderDrawColor(
      app_state->renderer,
      (std::sin(time) + 1) / 2.0 * 255,
      (std::sin(time / 2) + 1) / 2.0 * 255,
      (std::sin(time) * 2 + 1) / 2.0 * 255,
      SDL_ALPHA_OPAQUE);

  SDL_RenderClear(app_state->renderer);

  SDL_assert(app_state->bubble != nullptr);

  SDL_FRect src_rect{0, 0, 512, 512};
  SDL_FRect dest_rect{0, 0, 256, 256};

  if (!SDL_RenderTexture(
          app_state->renderer, app_state->bubble, &src_rect, &dest_rect)) {
    SDL_LogError(SDL_LOG_CATEGORY_CUSTOM, "SDL Error: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_RenderPresent(app_state->renderer);
  return app_state->app_quit;
}

void SDL_AppQuit(void* app_state_untyped, SDL_AppResult result) {
  auto* app_state = reinterpret_cast<AppState*>(app_state_untyped);

  if (app_state != nullptr) {
    SDL_DestroyRenderer(app_state->renderer);
    SDL_DestroyWindow(app_state->window);
    SDL_DestroyTexture(app_state->bubble);
    delete app_state;
  }

  SDL_Log("Application quit successfully!");
}
