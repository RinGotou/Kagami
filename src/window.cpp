#include "window.h"

namespace kagami {
#if not defined(_DISABLE_SDL_)
  /* base argument type */
  
  /* Under Construction */
  Message SDLInit(ObjectMap &p) {
    int result = SDL_Init(SDL_INIT_EVERYTHING);
    Message msg;
    if (result < 0) {
      msg = Message(kStrFalse);
    }
    else {
      msg = Message(kStrTrue);
    }
    return msg;
  }

  Message NewWindow(ObjectMap &p) {
    int width = stoi(p.Get<string>("width"));
    int height = stoi(p.Get<string>("height"));
    string title = p.Get<string>("title");
    auto x = p.Get<SDLWindowPos>("x");
    auto y = p.Get<SDLWindowPos>("y");
    Message msg;
    Object ret;

    auto window = SDL_CreateWindow(title.c_str(), x, y, 
      width, height, SDL_WINDOW_SHOWN);
    auto render = SDL_CreateRenderer(window, -1, 
      SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    Window winbase(window, render);
    ret.Set(make_shared<Window>(winbase), kTypeIdSDLWindow, "", false);
    msg.SetObject(ret);

    return msg;
  }

  Message WindowPos(ObjectMap &p) {
    static map<string, SDLWindowPos> result_base = {
      SDLWindowPosArg("'undefined'", SDL_WINDOWPOS_UNDEFINED),
      SDLWindowPosArg("'centered'", SDL_WINDOWPOS_CENTERED)
    };

    string pos = p.Get<string>("pos");
    SDLWindowPos pos_value;
    Message msg;

    auto it = result_base.find(pos);
    if (it != result_base.end()) {
      Object ret;
      ret.Set(make_shared<SDLWindowPos>(it->second), kTypeIdSDLWindowPos, "", false);
      msg.SetObject(ret);
    }
    else {
      msg = Message(kStrFatalError, kCodeIllegalParm, "Unknown Argument - " + pos + ".");
    }

    return msg;
  }

  Message NewTextureFromImage(ObjectMap &p) {
    string path = p.Get<string>("image_path");
    Message msg;

    return msg;
  }

  

  /* There is testbench for SDL2.You can just ignore them or disable them. */
  Message SDLTest(ObjectMap &p) {
    Message msg;
    int width = stoi(p.Get<string>("width"));
    int height = stoi(p.Get<string>("height"));
    string title = p.Get<string>("title");
    string path = p.Get<string>("path");

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      msg = Message(kStrFalse);
    }
    else {
      auto window = SDL_CreateWindow(title.c_str(), 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        width, height, SDL_WINDOW_SHOWN);
      auto render = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      auto image = SDL_LoadBMP(path.c_str());
      auto texture = SDL_CreateTextureFromSurface(render, image);
      SDL_RenderClear(render);
      SDL_RenderCopy(render, texture, nullptr, nullptr);
      SDL_RenderPresent(render);
      SDL_Delay(10000);
      SDL_DestroyTexture(texture);
      SDL_DestroyRenderer(render);
      SDL_DestroyWindow(window);
      SDL_Quit();

    }
    return msg;
  }

  void SetupSDLType() {

  }

  void LoadSDLStuff() {
    entry::AddEntry(Entry(SDLTest, kCodeNormalParm, "width|height|title|path", "SDLTestLoad"));
  }
#endif
}