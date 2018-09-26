#pragma once
#include "machine.h"
#if not defined(_DISABLE_SDL_)
#include <SDL.h>
#endif

namespace kagami {
#if not defined(_DISABLE_SDL_)
  class Window {
  private:
    SDL_Window *window;
    SDL_Surface *screenSurface;
  public:
    Window(SDL_Window *window, SDL_Surface *screenSurface) {
      this->window = window;
      this->screenSurface = screenSurface;
    }
    SDL_Window *Get() {
      return window;
    }
    SDL_Surface *GetSurface() {
      return screenSurface;
    }
  };
#endif
}
