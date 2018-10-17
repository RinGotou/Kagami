#pragma once
#include "machine.h"
#if not defined(_DISABLE_SDL_)
#include <SDL.h>
#include <SDL_image.h>
#endif

namespace kagami {
#if not defined(_DISABLE_SDL_)
  const string kSDLWindowMethods = "clear|copy|present";
  const string kSDLTexturemethods = "";
  const string kTypeIdSDLWindowPos = "SDLWindowPos";
  const string kTypeIdSDLWindow = "SDLWindow";
  const string kTypeIdSDLTexture = "SDLTexture";

  class Window {
  private:
    SDL_Window *window_;
    SDL_Renderer *render_;
  public:
    Window(SDL_Window *window, SDL_Renderer *render) {
      window_ = window;
      render_ = render;
    }
    SDL_Window *GetWindow() {
      return window_;
    }
    SDL_Renderer *GetRenderer() {
      return render_;
    }
  };

  using SDLWindowPos = unsigned int;
  using SDLWindowPosArg = pair<string, SDLWindowPos>;

  struct SDLTexture {
    SDL_Texture *texture;
  };

#endif
}
