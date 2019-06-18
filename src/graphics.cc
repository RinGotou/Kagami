#include "graphics.h"

namespace kagami {
  Message NewDisplayWindow(ObjectMap &p) {
    auto &width = p.Cast<int64_t>("width");
    auto &height = p.Cast<int64_t>("height");
    dawn::WindowOption option;

    option.width = static_cast<int>(width);
    option.height = static_cast<int>(height);

    dawn::ManagedWindow window = make_shared<dawn::BasicWindow>(option);

    return Message().SetObject(Object(window, kTypeIdWindow));
  }

  Message WindowSetBackground(ObjectMap &p) {
    auto &path = p.Cast<string>("path");
    auto &image_type = p.Cast<dawn::ImageType>("type");
    auto &window = p.Cast<dawn::BasicWindow>(kStrMe);
    auto renderer = window.GetRenderer();

    dawn::Texture backgroud_data(path, image_type, renderer);

    if (backgroud_data.Get() == nullptr) {
      return Message(kCodeIllegalParam, SDL_GetError(), kStateError);
    }
    
    window.Clear();
    window.Copy(backgroud_data);
    window.Present();

    return Message();
  }

  Message WindowAddImage(ObjectMap &p) {
    auto &path = p.Cast<string>("path");
    auto &image_type = p.Cast<dawn::ImageType>("type");
    auto x = static_cast<int>(p.Cast<int64_t>("x"));
    auto y = static_cast<int>(p.Cast<int64_t>("y"));
    auto &window = p.Cast<dawn::BasicWindow>(kStrMe);
    auto renderer = window.GetRenderer();

    dawn::Texture image_data(path, image_type, renderer);
    auto rect = dawn::ProduceRect(x, y, image_data.GetWidth(), image_data.GetHeight());

    if (image_data.Get() == nullptr) {
      return Message(kCodeIllegalParam, SDL_GetError(), kStateError);
    }

    window.Copy(image_data, nullptr, &rect);
    window.Present();

    return Message();
  }

  Message WindowSetText(ObjectMap &p) {
    auto text = p.Cast<string>("text");
    auto x = static_cast<int>(p.Cast<int64_t>("x"));
    auto y = static_cast<int>(p.Cast<int64_t>("y"));
    auto &window = p.Cast<dawn::BasicWindow>(kStrMe);
    auto &font = p.Cast<dawn::Font>("font");
    auto renderer = window.GetRenderer();
    auto &color = p.Cast<dawn::ColorValue>("color");

    dawn::Texture text_data(text, font, renderer, color);
    auto rect = dawn::ProduceRect(x, y, text_data.GetWidth(), text_data.GetHeight());

    if (text_data.Get() == nullptr) {
      return Message(kCodeIllegalParam, SDL_GetError(), kStateError);
    }

    window.Copy(text_data, nullptr, &rect);
    window.Present();

    return Message();
  }

  Message WindowWaiting(ObjectMap &p) {
    auto &window = p.Cast<dawn::BasicWindow>(kStrMe);
    bool exit = false;
    bool first_time = true;
    SDL_Event e;
    auto id = window.GetId();

    while (!exit) {
      while (SDL_PollEvent(&e) != 0) {
        if (e.window.windowID == id) {
          if (e.type == SDL_WINDOWEVENT && e.window.event == SDL_WINDOWEVENT_CLOSE) {
            exit = true;
          }
        }
      }
    }

    return Message();
  }

  Message WindowClear(ObjectMap &p) {
    auto &window = p.Cast<dawn::BasicWindow>(kStrMe);

    window.Clear();

    return Message();
  }

  Message WindowSetDrawColor(ObjectMap &p) {
    auto &window = p.Cast<dawn::BasicWindow>(kStrMe);
    auto &color = p.Cast<dawn::ColorValue>("color");
    window.SetDrawColor(color.r, color.g, color.b, color.a);

    return Message();
  }

  Message NewFont(ObjectMap &p) {
    auto size = static_cast<int>(p.Cast<int64_t>("size"));
    auto &path = p.Cast<string>("path");

    dawn::ManagedFont font = make_shared<dawn::Font>(path, size);

    if (font->Get() == nullptr) {
      return Message(kCodeIllegalParam, SDL_GetError(), kStateError);
    }

    return Message().SetObject(Object(font, kTypeIdFont));
  }

  Message NewColorValue(ObjectMap &p) {
    auto r = static_cast<int>(p.Cast<int64_t>("r"));
    auto g = static_cast<int>(p.Cast<int64_t>("g"));
    auto b = static_cast<int>(p.Cast<int64_t>("b"));
    auto a = static_cast<int>(p.Cast<int64_t>("a"));

    return Message().SetObject(Object(dawn::ColorValue(r, g, b, a), kTypeIdColorValue));
  }

  void InitWindowComponents() {
    using management::type::ObjectTraitsSetup;
    using namespace management;

    ObjectTraitsSetup(kTypeIdWindow, ShallowDelivery, PointerHasher)
      .InitConstructor(
        FunctionImpl(NewDisplayWindow, "width|height", "window")
      )
      .InitMethods(
        {
          FunctionImpl(WindowSetBackground,"path|type","set_background"),
          FunctionImpl(WindowAddImage, "path|type|x|y","add_image"),
          FunctionImpl(WindowSetText, "text|font|x|y|color","set_text"),
          FunctionImpl(WindowWaiting, "", "waiting"),
          FunctionImpl(WindowClear, "", "clear"),
          FunctionImpl(WindowSetDrawColor, "color", "set_draw_color")
        }
    );

    ObjectTraitsSetup(kTypeIdFont, ShallowDelivery, PointerHasher)
      .InitConstructor(
        FunctionImpl(NewFont, "size|path", "font")
    );

    ObjectTraitsSetup(kTypeIdColorValue, PlainDeliveryImpl<dawn::ColorValue>)
      .InitConstructor(
        FunctionImpl(NewColorValue, "r|g|b|a", "color")
    );

    CreateConstantObject(kStrImageJPG, Object(int64_t(dawn::kImageJPG), kTypeIdInt));
    CreateConstantObject(kStrImagePNG, Object(int64_t(dawn::kImagePNG), kTypeIdInt));
    CreateConstantObject(kStrImageTIF, Object(int64_t(dawn::kImageTIF), kTypeIdInt));
    CreateConstantObject(kStrImageWEBP, Object(int64_t(dawn::kImageWEBP), kTypeIdInt));

    CreateConstantObject(kStrKeycodeUp, Object(int64_t(SDLK_UP), kTypeIdInt));
    CreateConstantObject(kStrKeycodeDown, Object(int64_t(SDLK_DOWN), kTypeIdInt));
    CreateConstantObject(kStrKeycodeLeft, Object(int64_t(SDLK_LEFT), kTypeIdInt));
    CreateConstantObject(kStrKeycodeRight, Object(int64_t(SDLK_RIGHT), kTypeIdInt));
    CreateConstantObject(kStrKeycodeReturn, Object(int64_t(SDLK_RETURN), kTypeIdInt));

    CreateConstantObject(kStrEventKeydown, Object(int64_t(SDL_KEYDOWN), kTypeIdInt));
    CreateConstantObject(kStrEventWindowState, Object(int64_t(SDL_WINDOWEVENT), kTypeIdInt));
  }
}