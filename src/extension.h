#pragma once
#include "function.h"

namespace kagami {
  //TODO:External Type Facility
  using ExternalDelivery = void *(*)(void *);

  enum ExtActivityReturnType {
    kExtUnsupported         = -1,
    kExtTypeNull            = 0,
    kExtTypeInt             = 1,
    kExtTypeFloat           = 2,
    kExtTypeBool            = 3,
    kExtTypeString          = 4,
    kExtTypeWideString      = 5,
    kExtTypeFunctionPointer = 6,
    kExtTypeObjectPointer   = 7,
    kExtTypeArray           = 8,
    kExtCustomTypes         = 100
  };

  const unordered_map<string, ExtActivityReturnType> kExtTypeMatcher = {
    make_pair(kTypeIdNull, kExtTypeNull),
    make_pair(kTypeIdInt, kExtTypeInt),
    make_pair(kTypeIdFloat, kExtTypeFloat),
    make_pair(kTypeIdBool, kExtTypeBool),
    make_pair(kTypeIdString, kExtTypeString),
    make_pair(kTypeIdWideString, kExtTypeWideString),
    make_pair(kTypeIdFunctionPointer, kExtTypeFunctionPointer),
    make_pair(kTypeIdObjectPointer, kExtTypeObjectPointer),
    make_pair(kTypeIdArray, kExtTypeArray)
  };

  extern "C" struct Descriptor {
    void *ptr;
    ExtActivityReturnType type;
  };

  using DescriptorFetcher = int(*)(Descriptor *, void *, const char *);

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
