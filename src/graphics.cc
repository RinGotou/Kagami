#include "graphics.h"

#ifndef _DISABLE_SDL_
namespace kagami {
  //limit:2
	Message NewElement(ObjectMap& p) {
    auto &texture = p.Cast<dawn::Texture>("texture");
    auto &dest = p.Cast<SDL_Rect>("dest");
    auto &src_obj = p["src"];
    auto src = src_obj.Null() ?
      dawn::ProduceRect(0, 0, texture.GetWidth(), texture.GetHeight()) :
      src_obj.Cast<SDL_Rect>();
    dawn::Element element(texture, src, dest);

    return Message().SetObject(Object(element, kTypeIdElement));
	}

	Message ElementGetSrcInfo(ObjectMap &p) {
    auto &element = p.Cast<dawn::Element>(kStrMe);
    return Message().SetObject(Object(element.GetSrcInfo(), kTypeIdRectangle));
	}

	Message ElementGetDestInfo(ObjectMap &p) {
    auto &element = p.Cast<dawn::Element>(kStrMe);
    return Message().SetObject(Object(element.GetDestInfo(), kTypeIdRectangle));
	}

	Message ElementSetPriority(ObjectMap &p) {
    auto &element = p.Cast<dawn::Element>(kStrMe);
    auto &priority = p.Cast<int64_t>("priority");
    return Message().SetObject(element.SetPriority(int(priority)));
	}

	Message ElementGetPriority(ObjectMap &p) {
    auto &element = p.Cast<dawn::Element>(kStrMe);
    return Message().SetObject(int64_t(element.GetPriority()));
	}

  Message ElementSetDest(ObjectMap &p) {
    auto &element = p.Cast<dawn::Element>(kStrMe);
    auto &new_dest = p.Cast<SDL_Rect>("dest");
    auto &dest = element.GetDestInfo();
    dest.x = new_dest.x;
    dest.y = new_dest.y;
    return Message();
  }

  Message ElementSetSrc(ObjectMap &p) {
    auto &element = p.Cast<dawn::Element>(kStrMe);
    auto &new_src = p.Cast<SDL_Rect>("src");
    auto &src = element.GetSrcInfo();
    src.x = new_src.x;
    src.y = new_src.y;
    return Message();
  }

  Message NewWindow(ObjectMap &p) {
    auto &width = p.Cast<int64_t>("width");
    auto &height = p.Cast<int64_t>("height");
    dawn::WindowOption option;

    option.width = static_cast<int>(width);
    option.height = static_cast<int>(height);

    dawn::ManagedPlainWindow window = 
      make_shared<dawn::PlainWindow>(option);

    return Message().SetObject(Object(window, kTypeIdWindow));
  }

  Message WindowAddElement(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &element = p.Cast<dawn::Element>("element");

    return Message().SetObject(window.AddElement(id, element));
  }

  Message WindowSetElementPosition(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &point = p.Cast<SDL_Point>("point");

    return Message().SetObject(window.SetElementPosition(id, point));
  }

  Message WindowGetElementPosition(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(Object(window.GetElementPosition(id), kTypeIdPoint));
  }

  Message WindowSetElementSize(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &width = p.Cast<int64_t>("width");
    auto &height = p.Cast<int64_t>("height");

    return Message().SetObject(window.SetElementSize(id, int(width), int(height)));
  }

  Message WindowSetElementCropper(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("Id");
    auto &cropper = p.Cast<SDL_Rect>("cropper");

    return Message().SetObject(window.SetElementCropper(id, cropper));
  }

  Message WindowElementInRange(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &point = p.Cast<SDL_Point>("point");

    return Message().SetObject(window.ElementInRange(id, point));
  }

  Message WindowFindElementByPoint(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &point = p.Cast<SDL_Point>("point");
    auto *named_element = window.FindElementByPoint(point);
    string result = "";

    if (named_element != nullptr) {
      result = named_element->first;
    }

    return Message().SetObject(result);
  }

  Message WindowDisposeElement(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(window.DisposeElement(id));
  }

  Message WindowSetElementOnTop(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(window.SetElementOnTop(id));
  }

  Message WindowSetElementOnBottom(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(window.SetElementOnBottom(id));
  }

  Message WindowDraw(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    return Message().SetObject(window.DrawElements());
  }

  Message WindowSetBackground(ObjectMap &p) {
    auto &path = p.Cast<string>("path");
    auto &image_type = p.Cast<dawn::ImageType>("type");
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto renderer = window.GetRenderer();

    dawn::Texture background_data(path, image_type, renderer);

    if (background_data.Get() == nullptr) {
      return Message(kCodeIllegalParam, SDL_GetError(), kStateError);
    }
    
    window.Clear();
    window.Copy(background_data);
    window.Present();

    return Message();
  }

  Message WindowAddImage(ObjectMap &p) {
    auto &path = p.Cast<string>("path");
    auto &image_type = p.Cast<dawn::ImageType>("type");
    auto point = p.Cast<SDL_Point>("point");
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto renderer = window.GetRenderer();

    dawn::Texture image_data(path, image_type, renderer);
    auto rect = dawn::ProduceRect(point.x, point.y, image_data.GetWidth(), image_data.GetHeight());

    if (image_data.Get() == nullptr) {
      return Message(kCodeIllegalParam, SDL_GetError(), kStateError);
    }

    window.Copy(image_data, nullptr, &rect);
    window.Present();

    return Message();
  }

  Message WindowSetText(ObjectMap &p) {
    auto text = p.Cast<string>("text");
    auto point = p.Cast<SDL_Point>("point");
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &font = p.Cast<dawn::Font>("font");
    auto renderer = window.GetRenderer();
    auto &color = p.Cast<dawn::ColorValue>("color");

    dawn::Texture text_data(text, font, renderer, color);
    auto rect = dawn::ProduceRect(point.x, point.y, text_data.GetWidth(), text_data.GetHeight());

    if (text_data.Get() == nullptr) {
      return Message(kCodeIllegalParam, SDL_GetError(), kStateError);
    }

    window.Copy(text_data, nullptr, &rect);
    window.Present();

    return Message();
  }

  //Limit:1
  Message WindowCopy(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &texture = p.Cast<dawn::Texture>("texture");
    auto &src_rect_obj = p["src_rect"];
    auto &dest_rect_obj = p["dest_rect"];
    SDL_Rect *src_rect = src_rect_obj.Null() ? 
      nullptr : &src_rect_obj.Cast<SDL_Rect>();
    SDL_Rect *dest_rect = dest_rect_obj.Null() ?
      nullptr : &dest_rect_obj.Cast<SDL_Rect>();
    bool result = window.Copy(texture, src_rect, dest_rect);
    window.DrawElements();
    return Message().SetObject(result);
  }

  Message WindowWaiting(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
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
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);

    window.Clear();

    return Message();
  }

  Message WindowSetDrawColor(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &color = p.Cast<dawn::ColorValue>("color");
    window.SetDrawColor(color.r, color.g, color.b, color.a);

    return Message();
  }

  Message WindowInRange(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &rect = p.Cast<SDL_Rect>("rect");
    auto &point = p.Cast<SDL_Point>("point");

    return Message().SetObject(window.InRange(rect, point));
  }

  Message WindowRealTimeRefreshingMode(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &value = p.Cast<bool>("value");
    window.RealTimeRefreshingMode(value);
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

  Message NewRectangle(ObjectMap &p) {
    auto x = static_cast<int>(p.Cast<int64_t>("x"));
    auto y = static_cast<int>(p.Cast<int64_t>("y"));
    auto w = static_cast<int>(p.Cast<int64_t>("width"));
    auto h = static_cast<int>(p.Cast<int64_t>("height"));

    auto rect = dawn::ProduceRect(x, y, w, h);

    return Message().SetObject(Object(rect, kTypeIdRectangle));
  }

  Message NewPoint(ObjectMap &p) {
    auto x = static_cast<int>(p.Cast<int64_t>("x"));
    auto y = static_cast<int>(p.Cast<int64_t>("y"));

    auto point = dawn::ProducePoint(x, y);

    return Message().SetObject(Object(point, kTypeIdPoint));
  }

  Message PointGetX(ObjectMap &p) {
    auto &point = p.Cast<SDL_Point>(kStrMe);
    int64_t x = point.x;
    return Message().SetObject(x);
  }

  Message PointGetY(ObjectMap &p) {
    auto &point = p.Cast<SDL_Point>(kStrMe);
    int64_t y = point.y;
    return Message().SetObject(y);
  }

  Message NewTexture(ObjectMap &p) {
    return Message().SetObject(Object(dawn::Texture(), kTypeIdTexture));
  }

  //Limit:3
  Message TextureInitFromImage(ObjectMap &p) {
    auto &texture = p.Cast<dawn::Texture>(kStrMe);
    auto &image_path = p.Cast<string>("path");
    auto &type = p.Cast<dawn::ImageType>("type");
    auto &window = p.Cast<dawn::PlainWindow>("window");
    auto &color_key = p["color_key"];
    bool result = false;

    if (color_key.Null()) {
      result = texture.Init(image_path, type, window.GetRenderer());
    }
    else {
      //TODO:Remove redundant argument from init
      auto &color_value = color_key.Cast<dawn::ColorValue>();
      result = texture.Init(image_path, type, window.GetRenderer(), true, color_value);
    }

    return Message().SetObject(result);
  }

  Message TextureInitFromText(ObjectMap &p) {
    auto &texture = p.Cast<dawn::Texture>(kStrMe);
    auto &text = p.Cast<string>("text");
    auto &font = p.Cast<dawn::Font>("font");
    auto &window = p.Cast<dawn::PlainWindow>("window");
    auto &color_key = p.Cast<dawn::ColorValue>("color_key");
    bool result = texture.Init(text, font, window.GetRenderer(), color_key);
    return Message().SetObject(result);
  }

  Message TextureGood(ObjectMap &p) {
    auto &texture = p.Cast<dawn::Texture>(kStrMe);
    return Message().SetObject(texture.Get() != nullptr);
  }

  Message TextureHeight(ObjectMap &p) {
    auto &texture = p.Cast<dawn::Texture>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(texture.GetHeight()));
  }

  Message TextureWidth(ObjectMap &p) {
    auto &texture = p.Cast<dawn::Texture>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(texture.GetWidth()));
  }

  Message WindowEventGetType(ObjectMap &p) {
    auto &obj = p.Cast<SDL_WindowEvent>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(obj.event));
  }

  Message WindowEventGetData1(ObjectMap &p) {
    auto &obj = p.Cast<SDL_WindowEvent>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(obj.data1));
  }

  Message WindowEventGetData2(ObjectMap &p) {
    auto &obj = p.Cast<SDL_WindowEvent>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(obj.data2));
  }

  void CreateImageTypeMapping() {
    using namespace management;
    CreateConstantObject(kStrImageJPG, Object(int64_t(dawn::kImageJPG), kTypeIdInt));
    CreateConstantObject(kStrImagePNG, Object(int64_t(dawn::kImagePNG), kTypeIdInt));
    CreateConstantObject(kStrImageTIF, Object(int64_t(dawn::kImageTIF), kTypeIdInt));
    CreateConstantObject(kStrImageWEBP, Object(int64_t(dawn::kImageWEBP), kTypeIdInt));
  }

  void CreateKeyMapping() {
    using namespace management;
#define CREATE_KEYCODE(_Str, _Code) \
    CreateConstantObject(_Str, Object(int64_t(_Code), kTypeIdInt))

    CREATE_KEYCODE(kStrKeycodeUp, SDLK_UP);
    CREATE_KEYCODE(kStrKeycodeDown, SDLK_DOWN);
    CREATE_KEYCODE(kStrKeycodeLeft, SDLK_LEFT);
    CREATE_KEYCODE(kStrKeycodeRight, SDLK_RIGHT);
    CREATE_KEYCODE(kStrKeycodeReturn, SDLK_RETURN);

#undef CREATE_KEYODE
  }

  void CreateWindowStateMapping() {
    using namespace management;
    CreateConstantObject(kStrWindowClosed, Object(int64_t(SDL_WINDOWEVENT_CLOSE), kTypeIdInt));
    CreateConstantObject(kStrWindowMinimized, Object(int64_t(SDL_WINDOWEVENT_MINIMIZED), kTypeIdInt));
    CreateConstantObject(kStrWindowRestored, Object(int64_t(SDL_WINDOWEVENT_RESTORED), kTypeIdInt));
    CreateConstantObject(kStrWindowMouseEnter, Object(int64_t(SDL_WINDOWEVENT_ENTER), kTypeIdInt));
    CreateConstantObject(kStrWindowMouseLeave, Object(int64_t(SDL_WINDOWEVENT_LEAVE), kTypeIdInt));
    CreateConstantObject(kStrWindowMoved, Object(int64_t(SDL_WINDOWEVENT_MOVED), kTypeIdInt));
  }

  void CreateMouseButtonMapping() {
    using namespace management;
#define CREATE_BTN_CODE(_Str, _Code) \
    CreateConstantObject(_Str, Object(int64_t(_Code), kTypeIdInt))

    CREATE_BTN_CODE(kStrMouseLeft, SDL_BUTTON_LEFT);
    CREATE_BTN_CODE(kStrMouseMiddle, SDL_BUTTON_MIDDLE);
    CREATE_BTN_CODE(kStrMouseRight, SDL_BUTTON_RIGHT);

#undef CREATE_BTN_CODE
  }

  void CreateEventTypeId() {
    using namespace management;
#define CREATE_EVENTID(_Str, _Code) \
    CreateConstantObject(_Str, Object(int64_t(_Code), kTypeIdInt))

    CREATE_EVENTID(kStrEventKeydown, SDL_KEYDOWN);
    CREATE_EVENTID(kStrEventWindowState, SDL_WINDOWEVENT);
    CREATE_EVENTID(kStrEventMouseDown, SDL_MOUSEBUTTONDOWN);
    CREATE_EVENTID(kStrEventMouseUp, SDL_MOUSEBUTTONUP);
    CREATE_EVENTID(kStrEventMouseMotion, SDL_MOUSEMOTION);

#undef CREATE_EVENTID
  }

  void InitWindowComponents() {
    using management::type::ObjectTraitsSetup;
    using namespace management;

    ObjectTraitsSetup(kTypeIdElement, PlainDeliveryImpl<dawn::Element>)
      .InitConstructor(
        FunctionImpl(NewElement, "texture|dest|src", "element", kParamAutoFill).SetLimit(2)
      )
      .InitMethods(
        {
          FunctionImpl(ElementGetSrcInfo, "", "get_src"),
          FunctionImpl(ElementGetDestInfo, "", "get_dest"),
          FunctionImpl(ElementGetPriority, "", "get_priority"),
          FunctionImpl(ElementSetPriority, "priority", "set_priority"),
          FunctionImpl(ElementSetSrc, "src", "set_src"),
          FunctionImpl(ElementSetDest, "dest", "set_dest")
        }
    );

    ObjectTraitsSetup(kTypeIdWindow, ShallowDelivery, PointerHasher)
      .InitConstructor(
        FunctionImpl(NewWindow, "width|height", "window")
      )
      .InitMethods(
        {
          FunctionImpl(WindowAddElement, "id|element", "add_element"),
          FunctionImpl(WindowSetElementPosition, "id|point", "set_element_position"),
          FunctionImpl(WindowGetElementPosition, "id", "get_element_position"),
          FunctionImpl(WindowSetElementSize, "id|width|height", "set_element_size"),
          FunctionImpl(WindowSetElementCropper, "id|cropper", "set_element_cropper"),
          FunctionImpl(WindowElementInRange, "id|point", "element_in_range"),
          FunctionImpl(WindowFindElementByPoint, "point", "shoot"),
          FunctionImpl(WindowDisposeElement, "id", "dispose"),
          FunctionImpl(WindowSetElementOnTop, "id", "set_on_top"),
          FunctionImpl(WindowSetElementOnBottom, "id", "set_on_bottom"),
          FunctionImpl(WindowDraw, "", "draw"),
          FunctionImpl(WindowSetBackground,"path|type","set_background"),
          FunctionImpl(WindowAddImage, "path|type|point","add_image"),
          FunctionImpl(WindowSetText, "text|font|color|point","set_text"),
          FunctionImpl(WindowCopy, "texture|src_rect|dest_rect", "copy", kParamAutoFill).SetLimit(1),
          FunctionImpl(WindowWaiting, "", "waiting"),
          FunctionImpl(WindowClear, "", "clear"),
          FunctionImpl(WindowSetDrawColor, "color", "set_draw_color"),
          FunctionImpl(WindowInRange, "rect|point", "in_range"),
          FunctionImpl(WindowRealTimeRefreshingMode, "value", "real_time_refreshing")
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

    ObjectTraitsSetup(kTypeIdRectangle, PlainDeliveryImpl<SDL_Rect>)
      .InitConstructor(
        FunctionImpl(NewRectangle, "x|y|width|height", "rectangle")
    );

    ObjectTraitsSetup(kTypeIdPoint, PlainDeliveryImpl<SDL_Point>)
      .InitConstructor(
        FunctionImpl(NewPoint, "x|y", "point")
      )
      .InitMethods(
        {
          FunctionImpl(PointGetX, "", "get_x"),
          FunctionImpl(PointGetY, "", "get_y")
        }
    );

    ObjectTraitsSetup(kTypeIdTexture, ShallowDelivery)
      .InitConstructor(
        FunctionImpl(NewTexture, "", "texture")
      )
      .InitMethods(
        {
          FunctionImpl(TextureInitFromImage, "path|type|window|color_key", "from_image", kParamAutoFill).SetLimit(3),
          FunctionImpl(TextureInitFromText, "text|font|window|color_key", "from_text"),
          FunctionImpl(TextureGood, "", "good"),
          FunctionImpl(TextureHeight, "", "height"),
          FunctionImpl(TextureWidth, "", "width")
        }
    );

    ObjectTraitsSetup(kTypeIdWindowEvent, PlainDeliveryImpl<SDL_WindowEvent>)
      .InitMethods(
        {
          FunctionImpl(WindowEventGetType, "", "type"),
          FunctionImpl(WindowEventGetData1, "", "data1"),
          FunctionImpl(WindowEventGetData2, "", "data2")
        }
    );


    CreateImageTypeMapping();
    CreateKeyMapping();
    CreateWindowStateMapping();
    CreateMouseButtonMapping();
    CreateEventTypeId();
  }
}
#endif