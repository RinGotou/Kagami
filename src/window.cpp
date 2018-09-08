#include "window.h"

namespace kagami {
#if defined(_ENABLE_DEBUGGING_)
  /*There is testbench for SDL2.You can just ignore them or disable them.*/
  Message InitSDL(ObjectMap &p) {
    Message msg;
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
      msg.combo(kStrRedirect, kCodeSDLInfo, kStrFalse);
    }
    else {
      auto window = SDL_CreateWindow("Kagami Test Workbench", 
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 
        1280, 720, SDL_WINDOW_SHOWN);
      auto render = SDL_CreateRenderer(window, -1, 
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
      auto image = SDL_LoadBMP(R"(C:\workspace\img.bmp)");
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

  void LoadSDLStuff() {
    entry::AddEntry(Entry(InitSDL, kCodeNormalParm, "", "InitSDL"));
  }
#endif
}