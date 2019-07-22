#pragma once
#include "machine.h"

#ifndef _DISABLE_SDL_
namespace kagami {
  // Image type constant id
  const string kStrImageJPG = "kImageJPG";
  const string kStrImagePNG = "kImagePNG";
  const string kStrImageTIF = "kImageTIF";
  const string kStrImageWEBP = "kImageWEBP";

  // Keycode constant id
  const string kStrKeycodeUp = "kKeycodeUp";
  const string kStrKeycodeDown = "kKeycodeDown";
  const string kStrKeycodeLeft = "kKeycodeLeft";
  const string kStrKeycodeRight = "kKeycodeRight";
  const string kStrKeycodeReturn = "kKeycodeReturn";
  const string kStrKeycodeLCtrl = "kKeycodeLCtrl";
  const string kStrKeycodeRCtrl = "kKeycodeRCtrl";
  const string kStrKeycodeLShift = "kKeycodeLShift";
  const string kStrKeycodeRShift = "kKeycodeRShift";
  const string kStrKeycodeLAlt = "kKeycodeLAlt";
  const string kStrKeycodeRAlt = "kKeycodeRAlt";

  // Mouse button constant id
  const string kStrMouseLeft = "kMouseLeft";
  const string kStrMouseMiddle = "kMouseMiddle";
  const string kStrMouseRight = "kMouseRight";

  // For CommandHandle
  const string kStrEventKeydown = "kEventKeydown";
  const string kStrEventWindowState = "kEventWindowState";
  const string kStrEventMouseDown = "kEventMouseDown";
  const string kStrEventMouseUp = "kEventMouseUp";
  const string kStrEventMouseMotion = "kEventMouseMotion";
  const string kStrEventMouseWheel = "kEventMouseWheel";

  // Window state constant id
  // "Maximized" and "Resized" is not needed until resizable window is implemented.
  const string kStrWindowClosed = "kWindowClosed";
  const string kStrWindowMinimized = "kWindowMinimized";
  const string kStrWindowRestored = "kWindowRestored";
  const string kStrWindowMouseEnter = "kWindowMouseEnter";
  const string kStrWindowMouseLeave = "kWindowMouseLeave";
  const string kStrWindowMoved = "kWindowMoved";
}
#endif
