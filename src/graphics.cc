#include "graphics.h"

namespace kagami {
  //limit:2
  Message NewElement(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("texture", kTypeIdTexture),
        Expect("dest", kTypeIdRectangle),
        Expect("src", kTypeIdRectangle)
      }, p, { "src" }
    );

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &texture = p.Cast<dawn::Texture>("texture");
    auto &dest = p.Cast<SDL_Rect>("dest");
    auto &src_obj = p["src"];
    auto src = src_obj.Null() ?
      SDL_Rect{ 0, 0, texture.GetWidth(), texture.GetHeight() } :
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
    auto tc = TypeChecking({ Expect("priority", kTypeIdInt) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &element = p.Cast<dawn::Element>(kStrMe);
    auto &priority = p.Cast<int64_t>("priority");
    return Message().SetObject(element.SetPriority(int(priority)));
  }

  Message ElementGetPriority(ObjectMap &p) {
    auto &element = p.Cast<dawn::Element>(kStrMe);
    return Message().SetObject(int64_t(element.GetPriority()));
  }

  Message ElementSetDest(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("dest", kTypeIdRectangle) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &element = p.Cast<dawn::Element>(kStrMe);
    auto &new_dest = p.Cast<SDL_Rect>("dest");
    auto &dest = element.GetDestInfo();
    dest.x = new_dest.x;
    dest.y = new_dest.y;
    return Message();
  }

  Message ElementSetSrc(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("src", kTypeIdRectangle) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &element = p.Cast<dawn::Element>(kStrMe);
    auto &new_src = p.Cast<SDL_Rect>("src");
    auto &src = element.GetSrcInfo();
    src.x = new_src.x;
    src.y = new_src.y;
    return Message();
  }

  Message ElementSetTexture(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("texture", kTypeIdTexture) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &element = p.Cast<dawn::Element>(kStrMe);
    auto &texture = p.Cast<dawn::Texture>("texture");
    element.SetTexture(texture.Get());

    return Message();
  }

  Message NewWindow(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("width", kTypeIdInt),
        Expect("height", kTypeIdInt)
      }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

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
    auto tc = TypeChecking(
      {
        Expect("id", kTypeIdString),
        Expect("element", kTypeIdElement)
      }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &element = p.Cast<dawn::Element>("element");

    return Message().SetObject(window.AddElement(id, element));
  }

  Message WindowGetElementDestination(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("id", kTypeIdString) }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(Object(window.GetElementDestination(id), kTypeIdRectangle));
  }

  Message WindowSetElementDestination(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("id", kTypeIdString),
        Expect("dest", kTypeIdRectangle)
      }, p
    );

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &rect = p.Cast<SDL_Rect>("dest");

    return Message().SetObject(window.SetElementDestination(id, rect));
  }

  Message WindowSetElementPosition(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("id", kTypeIdString),
        Expect("point", kTypeIdPoint)
      }, p
    );

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &point = p.Cast<SDL_Point>("point");

    return Message().SetObject(window.SetElementPosition(id, point));
  }

  Message WindowGetElementPosition(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("id", kTypeIdString) }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(Object(window.GetElementPosition(id), kTypeIdPoint));
  }

  Message WindowSetElementSize(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("id", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &width = p.Cast<int64_t>("width");
    auto &height = p.Cast<int64_t>("height");

    return Message().SetObject(window.SetElementSize(id, int(width), int(height)));
  }

  Message WindowGetElementSize(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("id", kTypeIdString) }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(Object(window.GetElementSize(id), kTypeIdPoint));
  }

  Message WindowSetElementCropper(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("id", kTypeIdString),
        Expect("cropper", kTypeIdRectangle)
      }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &cropper = p.Cast<SDL_Rect>("cropper");

    return Message().SetObject(window.SetElementCropper(id, cropper));
  }

  Message WindowGetElementCropper(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("id", kTypeIdString) }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(Object(window.GetElementCropper(id), kTypeIdPoint));
  }

  Message WindowElementInRange(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("id", kTypeIdString),
        Expect("point", kTypeIdPoint)
      }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto &point = p.Cast<SDL_Point>("point");

    return Message().SetObject(window.ElementInRange(id, point));
  }

  Message WindowFindElementByPoint(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("point", kTypeIdPoint) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

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
    auto tc = TypeChecking({ Expect("id", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(window.DisposeElement(id));
  }

  Message WindowSetElementOnTop(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("id", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(window.SetElementOnTop(id));
  }

  Message WindowSetElementOnBottom(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("id", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &id = p.Cast<string>("id");

    return Message().SetObject(window.SetElementOnBottom(id));
  }

  Message WindowSetElementTexture(ObjectMap &p) {
    auto tc = TypeChecking({
      Expect("texture", kTypeIdTexture),
      Expect("id", kTypeIdString)
      }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &texture = p.Cast<dawn::Texture>("texture");
    auto &id = p.Cast<string>("id");

    return Message().SetObject(window.SetElementTexture(id, texture));
  }

  Message WindowGetId(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    return Message().SetObject(int64_t(window.GetId()));
  }

  Message WindowSetTitle(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("title", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    auto &title = p.Cast<string>("title");

    window.SetWindowTitle(title);

    return Message();
  }

  Message WindowDraw(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    return Message().SetObject(window.DrawElements());
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

  Message WindowShow(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    window.Show();
    return Message();
  }

  Message WindowHide(ObjectMap &p) {
    auto &window = p.Cast<dawn::PlainWindow>(kStrMe);
    window.Hide();
    return Message();
  }

  Message NewFont(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("path", kTypeIdString),
        Expect("size", kTypeIdInt),
      }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto size = static_cast<int>(p.Cast<int64_t>("size"));
    auto &path = p.Cast<string>("path");

    dawn::ManagedFont font = make_shared<dawn::Font>(path, size);

    if (font->Get() == nullptr) {
      return Message(SDL_GetError(), kStateError);
    }

    return Message().SetObject(Object(font, kTypeIdFont));
  }

  Message NewColorValue(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("r", kTypeIdInt), Expect("g", kTypeIdInt),
        Expect("b", kTypeIdInt), Expect("a", kTypeIdInt)
      }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto r = static_cast<int>(p.Cast<int64_t>("r"));
    auto g = static_cast<int>(p.Cast<int64_t>("g"));
    auto b = static_cast<int>(p.Cast<int64_t>("b"));
    auto a = static_cast<int>(p.Cast<int64_t>("a"));

    return Message().SetObject(Object(dawn::ColorValue(r, g, b, a), kTypeIdColorValue));
  }

  Message NewRectangle(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("x", kTypeIdInt), Expect("y", kTypeIdInt),
        Expect("width", kTypeIdInt), Expect("height", kTypeIdInt)
      }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto x = static_cast<int>(p.Cast<int64_t>("x"));
    auto y = static_cast<int>(p.Cast<int64_t>("y"));
    auto w = static_cast<int>(p.Cast<int64_t>("width"));
    auto h = static_cast<int>(p.Cast<int64_t>("height"));

    SDL_Rect rect{ x, y, w, h };

    return Message().SetObject(Object(rect, kTypeIdRectangle));
  }

  Message NewPoint(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("x", kTypeIdInt), Expect("y", kTypeIdInt) }, p);

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto x = static_cast<int>(p.Cast<int64_t>("x"));
    auto y = static_cast<int>(p.Cast<int64_t>("y"));

    SDL_Point point{ x, y };

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
    auto managed_texture = make_shared<dawn::Texture>();
    return Message().SetObject(Object(managed_texture, kTypeIdTexture));
  }

  //Limit:3
  Message TextureInitFromImage(ObjectMap &p) {
    auto tc = TypeChecking(
      {
        Expect("path", kTypeIdString),
        Expect("type", kTypeIdInt),
        Expect("window", kTypeIdWindow),
        Expect("color_key", kTypeIdColorValue)
      }, p, { "color_key" });

    if (TC_FAIL(tc)) return TC_ERROR(tc);

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
    auto tc = TypeChecking(
      {
        Expect("text", kTypeIdString),
        Expect("font", kTypeIdFont),
        Expect("window", kTypeIdWindow),
        Expect("color_key", kTypeIdColorValue),
        Expect("wrap_length", kTypeIdInt)
      }, p, { "wrap_length" });

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &texture = p.Cast<dawn::Texture>(kStrMe);
    auto &text = p.Cast<string>("text");
    auto &font = p.Cast<dawn::Font>("font");
    auto &window = p.Cast<dawn::PlainWindow>("window");
    auto &color_key = p.Cast<dawn::ColorValue>("color_key");
    auto wrap_length_obj = p["wrap_length"];
    bool result = false;

    result = texture.Init(text, font, window.GetRenderer(), color_key,
      Uint32(wrap_length_obj.Null() ? 0 : wrap_length_obj.Cast<int64_t>()));

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

  Message GetSDLError(ObjectMap &p) {
    return Message().SetObject(string(SDL_GetError()));
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
    CREATE_KEYCODE(kStrKeycodeLCtrl, SDLK_LCTRL);
    CREATE_KEYCODE(kStrKeycodeRCtrl, SDLK_RCTRL);
    CREATE_KEYCODE(kStrKeycodeLShift, SDLK_LSHIFT);
    CREATE_KEYCODE(kStrKeycodeRShift, SDLK_RSHIFT);
    CREATE_KEYCODE(kStrKeycodeLAlt, SDLK_LALT);
    CREATE_KEYCODE(kStrKeycodeRAlt, SDLK_RALT);
    CREATE_KEYCODE(kStrKeycodeTab, SDLK_TAB);
    CREATE_KEYCODE(kStrKeycodeCaps, SDLK_CAPSLOCK);

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
    CREATE_EVENTID(kStrEventKeyup, SDL_KEYUP);
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
          FunctionImpl(ElementSetDest, "dest", "set_dest"),
          FunctionImpl(ElementSetTexture, "texture", "set_texture")
        }
    );
    
    ObjectTraitsSetup(kTypeIdWindow, ShallowDelivery, PointerHasher)
      .InitConstructor(
        FunctionImpl(NewWindow, "width|height", "window")
      )
      .InitMethods(
        {
          FunctionImpl(WindowAddElement, "id|element", "add_element"),
          FunctionImpl(WindowGetElementDestination, "id", "get_element_dest"),
          FunctionImpl(WindowSetElementDestination, "id|dest", "set_element_dest"),
          FunctionImpl(WindowGetElementPosition, "id", "get_element_position"),
          FunctionImpl(WindowSetElementPosition, "id|point", "set_element_position"),
          FunctionImpl(WindowGetElementSize, "id", "get_element_size"),
          FunctionImpl(WindowSetElementSize, "id|width|height", "set_element_size"),
          FunctionImpl(WindowGetElementCropper, "id", "get_element_cropper"),
          FunctionImpl(WindowSetElementCropper, "id|cropper", "set_element_cropper"),
          FunctionImpl(WindowElementInRange, "id|point", "element_in_range"),
          FunctionImpl(WindowFindElementByPoint, "point", "shoot"),
          FunctionImpl(WindowDisposeElement, "id", "dispose"),
          FunctionImpl(WindowSetElementOnTop, "id", "set_on_top"),
          FunctionImpl(WindowSetElementOnBottom, "id", "set_on_bottom"),
          FunctionImpl(WindowSetElementTexture, "id|texture", "set_texture"),
          FunctionImpl(WindowGetId, "", "id"),
          FunctionImpl(WindowSetTitle, "title", "set_title"),
          FunctionImpl(WindowDraw, "", "draw"),
          FunctionImpl(WindowWaiting, "", "waiting"),
          FunctionImpl(WindowClear, "", "clear"),
          FunctionImpl(WindowSetDrawColor, "color", "set_draw_color"),
          FunctionImpl(WindowInRange, "rect|point", "in_range"),
          FunctionImpl(WindowRealTimeRefreshingMode, "value", "real_time_refreshing"),
          FunctionImpl(WindowShow, "", "show"),
          FunctionImpl(WindowHide, "", "hide")
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
          FunctionImpl(TextureInitFromText, "text|font|window|color_key|wrap_length", "from_text", kParamAutoFill).SetLimit(4),
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

    CreateImpl(FunctionImpl(GetSDLError, "", "SDL_error"));

    CreateImageTypeMapping();
    CreateKeyMapping();
    CreateWindowStateMapping();
    CreateMouseButtonMapping();
    CreateEventTypeId();

    EXPORT_CONSTANT(kTypeIdWindowEvent);
    EXPORT_CONSTANT(kTypeIdWindow);
    EXPORT_CONSTANT(kTypeIdElement);
    EXPORT_CONSTANT(kTypeIdFont);
    EXPORT_CONSTANT(kTypeIdTexture);
    EXPORT_CONSTANT(kTypeIdColorValue);
    EXPORT_CONSTANT(kTypeIdRectangle);
    EXPORT_CONSTANT(kTypeIdPoint);
    EXPORT_CONSTANT(kTypeIdExtension);
  }
}