#pragma once

#include <forge/support/sdl_support.h>

#include <SDL3/SDL.h>

/// The base class for all Forge games and is responsible for handling the
/// common application logic required for all games.
///
/// Games should derive from `Game`, and implement the `on_*` methods that are
/// listed below.
class Game {
public:
  /// Constructor.
  ///
  /// @param renderer Pointer to the `SDL_Renderer` for the main window.
  /// @param window  Pointer to the main SDL window.
  Game(unique_sdl_renderer_ptr renderer, unique_sdl_window_ptr window);

  /// Destructor.
  virtual ~Game();

  /// Initializes the game.
  SDL_AppResult init();

  /// Handle an event received from the host platform.
  SDL_AppResult handle_event(SDL_Event* event);

  /// Advance the game's simulation logic and rendering.
  SDL_AppResult iterate();

  /// Get the width of the main rendering window in pixel units.
  int pixel_width() const { return pixel_width_; }

  /// Get the height of the main rendering window in pixel units.
  int pixel_height() const { return pixel_height_; }

protected:
  /// Called at the end of the game initialization phase.
  virtual SDL_AppResult on_init();

  /// Called every update to allow a game to resond to player inputs.
  ///
  /// @param delta_s The amount of time that has elapsed in seconds since the
  ///                last call to this function.
  virtual SDL_AppResult on_input(float delta_s);

  /// Called to update the game's simulation logic on a fixed timestep
  /// frequency.
  ///
  /// @param delta_s The amount of time that has elapsed in seconds since the
  ///                last call to this function (always `kMsPerUpdate`).
  virtual SDL_AppResult on_update(float delta_s);

  /// Called to render the game's simulation state.
  ///
  /// @param extrapolation An interpolation value [0.0, 1.0) representing the
  ///                      time between the last update and the upcoming update.
  virtual SDL_AppResult on_render(float delta_s, float extrapolation);

  /// Called when the main render window is resized.
  virtual SDL_AppResult on_render_resized(int width, int height);

  /// Called when the mouse is clicked inside the main render window.
  virtual SDL_AppResult on_mouse_click(int mouse_x, int mouse_y);

  /// Called when a finger is starts touching inside the main render window.
  virtual SDL_AppResult on_touch_finger_down(int touch_x, int touch_y);

protected:
  /// The `SDL_Renderer` for the game's main window.
  unique_sdl_renderer_ptr renderer_;

  /// The game's main window.
  unique_sdl_window_ptr window_;

  /// True if the game should exit, false otherwise.
  bool quit_requested_ = false;

  /// Width of the main rendering window in pixels.
  int pixel_width_ = 0;

  /// Height of the main rendering window in pixels.
  int pixel_height_ = 0;

public:
  /// The number of milliseconds between game logic updates.
  static constexpr Uint64 kMsPerUpdate = 16; // about 60/sec.

private:
  /// The last time `on_iterate` was called.
  Uint64 previous_time_ms_ = 0;

  /// The amount of time that has elapsed since the last game logic update.
  /// This should never exceed `kMsPerUpdate`.
  Uint64 lag_time_ms_ = 0;
};