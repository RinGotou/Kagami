#include "window.h"

namespace kagami {
#if not defined(_DISABLE_SDL_)

  /* Under Construction */

  Message SDLCreateWindow(ObjectMap &p) {
    int w = stoi(p.Get<string>("width"));
    int h = stoi(p.Get<string>("height"));
    string title = p.Get<string>("title");
    Object ret;
    WindowBase win_base = make_shared<Window>();
    win_base->window = SDL_CreateWindow(title.c_str(),
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      w, h, SDL_WINDOW_SHOWN);
    win_base->render = SDL_CreateRenderer(win_base->window, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    ret.Set(win_base, kTypeIdSDLWindow);
    return Message().SetObject(ret);
  }
  
  Message SDLCreateTextureFormBMP(ObjectMap &p) {
    string image_path = p.Get<string>("path");
    Window &win = p.Get<Window>("win");
    Object ret;
    TextureBase texture_base = make_shared<Texture>();
    SDL_Surface *image = SDL_LoadBMP(image_path.c_str());
    texture_base->texture = SDL_CreateTextureFromSurface(win.render, image);
    SDL_FreeSurface(image);
    ret.Set(texture_base, kTypeIdSDLTexture);
    return Message().SetObject(ret);
  }
 

  Message SDLTestPresent(ObjectMap &p) {
    Texture &te = p.Get<Texture>("texture");
    Window &win = p.Get<Window>("win");
    SDL_RenderClear(win.render);
    SDL_RenderCopy(win.render, te.texture, NULL, NULL);
    SDL_RenderPresent(win.render);
    return Message();
  }

  Message SDLDelay(ObjectMap &p) {
    int time = stoi(p.Get<string>("time"));
    SDL_Delay(time);
    return Message();
  }
 
  void LoadSDLStuff() {
    using management::type::NewTypeSetup;
    using management::CreateInterface;

    /* For test only */
    NewTypeSetup(kTypeIdSDLWindow, FakeCopy)
      .InitConstructor(Interface(SDLCreateWindow, "width|height|title", "window"));

    NewTypeSetup(kTypeIdSDLTexture, FakeCopy);
    CreateInterface(Interface(SDLCreateTextureFormBMP, "win|path", "LoadBMP"));

    CreateInterface(Interface(SDLDelay, "time", "SDLDelay"));
    CreateInterface(Interface(SDLTestPresent, "texture|win", "Present"));
  }
#endif
}