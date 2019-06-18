#pragma once
#include "machine.h"

#ifndef _DISABLE_SDL_
namespace kagami {
  const string kTypeIdWindow = "window";
  const string kTypeIdFont = "font";
  const string kTypeIdColorValue = "color";
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
}
#endif
