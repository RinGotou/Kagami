#pragma once
#include "machine.h"

#ifndef _DISABLE_SDL_
namespace kagami {
  const string kTypeIdWindow = "window";
  const string kTypeIdFont = "font";
  const string kTypeIdTexture = "texture";
  const string kTypeIdColorValue = "color";
  const string kTypeIdRectangle = "rectangle";
  const string kTypeIdPoint = "point";
  const string kStrImageJPG = "kImageJPG";
  const string kStrImagePNG = "kImagePNG";
  const string kStrImageTIF = "kImageTIF";
  const string kStrImageWEBP = "kImageWEBP";
  const string kStrKeycodeUp = "kKeycodeUp";
  const string kStrKeycodeDown = "kKeycodeDown";
  const string kStrKeycodeLeft = "kKeycodeLeft";
  const string kStrKeycodeRight = "kKeycodeRight";
  const string kStrKeycodeReturn = "kKeycodeReturn";
  const string kStrEventKeydown = "kEventKeydown";
  const string kStrEventWindowState = "kEventWindowState";

  //Window state constant object id
  //"Maximized" is not needed until resizable window is implemented.

  //const string kStrWindowStateMaximized = "kWindowStateMaximized";
  const string kStrWindowMinimized = "kWindowMinimized";
  const string kStrWindowRestored = "kWindowRestored";
  const string kStrWindowMouseEnter = "kWindowMouseEnter";
  const string kStrWindowMouseLeave = "kWindowMouseLeave";
  const string kStrWindowMoved = "kWindowMoved";
}
#endif
