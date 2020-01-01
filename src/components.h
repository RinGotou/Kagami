#pragma once
#include "common.h"

namespace kagami {
  using ComponentLoader = void(*)();

  void InitConsoleComponents();
  void InitBaseTypes();
  void InitContainerComponents();
  void InitFunctionType();
  void InitStreamComponents();
  void InitSoundComponents();
  void InitWindowComponents();

  const ComponentLoader kEmbeddedComponents[] = {
    InitConsoleComponents,
    InitBaseTypes,
    InitContainerComponents,
    InitFunctionType,
    InitStreamComponents,
    InitSoundComponents,
    InitWindowComponents
  };
}
