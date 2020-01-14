#pragma once
#include "function.h"

namespace kagami {
  //TODO:External Type Facility
  using ExternalDelivery = void *(*)(void *);

  enum ExtActivityReturnType {
    kExtTypeNull       = 0,
    kExtTypeInt        = 1,
    kExtTypeFloat      = 2,
    kExtTypeBool       = 3,
    kExtTypeString     = 4,
    kExtTypeWideString = 5
  };

#ifdef _WIN32
  class Extension {
  protected:
    HMODULE ptr_;

    template <typename _FunctionPattern>
    _FunctionPattern FetchInterface(string id) {
      auto func_ptr = _FunctionPattern(GetProcAddress(ptr_, id.data()));
      return func_ptr;
    }
  public:
    ~Extension() { FreeLibrary(ptr_); }
    Extension() = delete;
    Extension(string path) : ptr_(nullptr) {
      wstring wstr = s2ws(path);
      ptr_ = LoadLibraryW(wstr.data());
    }

    ExtensionActivity FetchFunction(string id) {
      return FetchInterface<ExtensionActivity>(id);
    }

    ParameterInformer GetParameterInformer() {
      return FetchInterface<ParameterInformer>("kagami_ParameterInformer");
    }

    ExtensionLoader GetExtensionLoader() {
      return FetchInterface<ExtensionLoader>("kagami_LoadExtension");
    }

    bool Good() const { return ptr_ != nullptr; }
  };
#else
  class Extension {
  protected:
    void *ptr_;

    template <typename _FunctionPattern>
    _FunctionPattern FetchInterface(string id) {
      auto func_ptr = _FunctionPattern(dlsym(ptr_, id.data()));
      return func_ptr;
    }
  public:
    ~Extension() { dlclose(ptr_); }
    Extension() = delete;
    Extension(string path) : ptr_(dlopen(path.data(), RTLD_LAZY)) {}

    ExtensionActivity FetchFunction(string id) {
      return FetchInterface<ExtensionActivity>(id);
  }

    ParameterInformer GetParameterInformer() {
      return FetchInterface<ParameterInformer>("kagami_ParameterInformer");
    }

    ExtensionLoader GetExtensionLoader() {
      return FetchInterface<ExtensionLoader>("kagami_LoadExtension");
    }

    bool Good() const { return ptr_ != nullptr; }
  };
#endif

  using ManagedExtension = shared_ptr<Extension>;
}
