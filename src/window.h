#pragma once
#include "machine.h"
#include <SDL.h>

namespace kagami {
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
}
