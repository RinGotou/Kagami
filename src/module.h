#pragma once
#include "machine.h"

namespace kagami {
  using namespace management::plugin;
#ifdef _WIN32
  class ExternalPlugin {
  protected:
    HMODULE ptr_;
  public:
    ~ExternalPlugin() { FreeLibrary(ptr_); }
    ExternalPlugin() : ptr_(nullptr) {}
    ExternalPlugin(wstring path) : ptr_(LoadLibrary(path.data())) {}

    template <typename _TargetFunction>
    bool GetTargetInterface(_TargetFunction &func, wstring id) {
      func = static_cast<_TargetFunction>(GetProcAddress(ptr_, id.data()));
      if (func == nullptr) return false;
      return true;
    }
  };
#else

#endif
}