#pragma once
#include "module.h"


namespace kagami {
#if not defined(_DISABLE_SDL_)
  const string kSDLWindowMethods = "clear|copy|present";
  const string kSDLTexturemethods = "";
  const string kTypeIdSDLWindowPos = "SDLWindowPos";
  const string kTypeIdSDLWindow = "SDLWindow";
  const string kTypeIdSDLTexture = "SDLTexture";

  class Window {
  public:
    SDL_Window *window;
    SDL_Renderer *render;

    ~Window() {
      SDL_DestroyRenderer(render);
      SDL_DestroyWindow(window);
    }
  };

  class Texture {
  public:
    SDL_Texture *texture;

    ~Texture() {
      SDL_DestroyTexture(texture);
    }
  };

  using WindowBase = shared_ptr<Window>;
  using TextureBase = shared_ptr<Texture>;

#endif
}
