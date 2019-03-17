#include "dawn.window.h"

namespace dawn {
  SDL_Rect ProduceRect(int x, int y, int w, int h) {
    SDL_Rect result = { x,y,w,h };
    return result;
  }

  SDL_Point ProducePoint(int x, int y) {
    SDL_Point result = { x,y };
    return result;
  }

  FlipOption ProduceFlipOption(double angle, int x, int y, SDL_RendererFlip mode) {
    FlipOption result = { angle, mode, ProducePoint(x,y) };
    return result;
  }

  bool BasicWindow::Copy(Texture &texture, SDL_Rect *src_rect, SDL_Rect *dest_rect) {
    return SDL_RenderCopy(renderer_, texture.Get(), src_rect, dest_rect) == 0;
  }

  bool BasicWindow::Copy(Texture & texture, SDL_Rect * src_rect, SDL_Rect * dest_rect, 
    FlipOption option) {
    return SDL_RenderCopyEx(renderer_, texture.Get(), src_rect, dest_rect,
      option.angle_value, &option.point, option.mode) == 0;
  }

  bool BasicWindow::SetViewport(SDL_Rect pos) {
    return SDL_RenderSetViewport(renderer_, &pos) == 0;
  }

  SDL_Rect BasicWindow::GetViewport() {
    SDL_Rect result;
    SDL_RenderGetViewport(renderer_, &result);
    return result;
  }

  bool Texture::Init(string path, int type, SDL_Renderer *renderer,
    bool enable_colorkey, ColorValue key) {
    if (ptr_ != nullptr) {
      SDL_DestroyTexture(ptr_);
    }

    IMG_Init(type);
    SDL_Surface *surface = IMG_Load(path.c_str());

    if (surface == nullptr) return false;

    if (enable_colorkey) {
      uint32_t value;
      switch (key.type) {
      case kColorRGB:
        value = SDL_MapRGB(surface->format, key.r, key.g, key.b);
        break;
      case kColorRGBA:
        value = SDL_MapRGBA(surface->format, key.r, key.g, key.b, key.a);
        break;
      default:
        break;
      }

      if (SDL_SetColorKey(surface, 1, value) != 0) {
        return false;
      }
    }

    ptr_ = SDL_CreateTextureFromSurface(renderer, surface);
    width_ = surface->w;
    height_ = surface->h;

    SDL_FreeSurface(surface);

    return ptr_ != nullptr;
  }

  bool Texture::Init(string text, Font &font, SDL_Renderer *renderer, ColorValue color) {
    if (ptr_ != nullptr) {
      SDL_DestroyTexture(ptr_);
    }

    SDL_Surface *surface = nullptr;

    switch (color.type) {
    case kColorRGB:
      surface = TTF_RenderText_Solid(font.Get(), text.c_str(), color.Get());
      break;
    case kColorRGBA:
      surface = TTF_RenderText_Blended(font.Get(), text.c_str(), color.Get());
      break;
    default:
      break;
    }

    if (surface == nullptr) return false;

    ptr_ = SDL_CreateTextureFromSurface(renderer, surface);
    width_ = surface->w;
    height_ = surface->h;

    SDL_FreeSurface(surface);

    return ptr_ != nullptr;
  }
}