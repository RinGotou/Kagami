#include "window.h"

namespace kagami {
#if not defined(_DISABLE_SDL_)

  /* Under Construction */

  Message SDLCreateWindow(ObjectMap &p) {
    int w = stoi(p.Cast<string>("width"));
    int h = stoi(p.Cast<string>("height"));
    string title = p.Cast<string>("title");
    Object ret;
    WindowBase win_base = make_shared<Window>();
    win_base->window = SDL_CreateWindow(title.c_str(),
      SDL_WINDOWPOS_CENTERED,
      SDL_WINDOWPOS_CENTERED,
      w, h, SDL_WINDOW_SHOWN);
    win_base->render = SDL_CreateRenderer(win_base->window, -1,
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    ret.ManageContent(win_base, kTypeIdSDLWindow);
    return Message().SetObject(ret);
  }
  
  Message SDLCreateTextureFormBMP(ObjectMap &p) {
    string image_path = p.Cast<string>("path");
    Window &win = p.Cast<Window>("win");
    Object ret;
    TextureBase texture_base = make_shared<Texture>();
    SDL_Surface *image = SDL_LoadBMP(image_path.c_str());
    texture_base->texture = SDL_CreateTextureFromSurface(win.render, image);
    SDL_FreeSurface(image);
    ret.ManageContent(texture_base, kTypeIdSDLTexture);
    return Message().SetObject(ret);
  }
 

  Message SDLTestPresent(ObjectMap &p) {
    Texture &te = p.Cast<Texture>("texture");
    Window &win = p.Cast<Window>("win");
    SDL_RenderClear(win.render);
    SDL_RenderCopy(win.render, te.texture, NULL, NULL);
    SDL_RenderPresent(win.render);
    return Message();
  }

  Message SDLDelay(ObjectMap &p) {
    long time = stol(p.Cast<string>("time"));
    SDL_Delay(time);
    return Message();
  }
 
  void LoadSDLStuff() {
    using management::type::NewTypeSetup;
    using management::CreateNewInterface;

    /* For test only */
    NewTypeSetup(kTypeIdSDLWindow, FakeCopy)
      .InitConstructor(Interface(SDLCreateWindow, "width|height|title", "window"));

    NewTypeSetup(kTypeIdSDLTexture, FakeCopy);
    CreateNewInterface(Interface(SDLCreateTextureFormBMP, "win|path", "LoadBMP"));

    CreateNewInterface(Interface(SDLDelay, "time", "SDLDelay"));
    CreateNewInterface(Interface(SDLTestPresent, "texture|win", "Present"));
  }
#endif
}