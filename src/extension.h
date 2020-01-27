#pragma once
#include "function.h"

namespace kagami {
  //TODO:External Type Facility
  using ExternalDelivery = void *(*)(void *);

  enum ObjectType {
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

  struct _UnsupportedObjectType {};
  struct NullObjectType {};

  template <ObjectType _TypeCode>
  struct _ObjectTypeTrait {
    using Type = _UnsupportedObjectType;
  };

  template <typename _Type>
  struct _RevObjectTypeTrait {
    static const ObjectType type = kExtUnsupported;
  };

#define INIT_OBJECT_TYPE(_TypeCode, _Type)                   \
  template <>                                                \
  struct _ObjectTypeTrait<_TypeCode> { using Type = _Type; };

  INIT_OBJECT_TYPE(kExtTypeNull, NullObjectType)
    INIT_OBJECT_TYPE(kExtTypeInt, int64_t)
    INIT_OBJECT_TYPE(kExtTypeFloat, double)
    INIT_OBJECT_TYPE(kExtTypeBool, int)
    INIT_OBJECT_TYPE(kExtTypeString, string)
    INIT_OBJECT_TYPE(kExtTypeWideString, wstring)
    INIT_OBJECT_TYPE(kExtTypeFunctionPointer, GenericFunctionPointer)
    INIT_OBJECT_TYPE(kExtTypeObjectPointer, GenericPointer)

#undef INIT_OBJECT_TYPE
#define INIT_REV_OBJECT_TYPE(_Type, _TypeCode)               \
  template <>                                                \
  struct _RevObjectTypeTrait<_Type> { static const ObjectType type = _TypeCode; };

    INIT_REV_OBJECT_TYPE(NullObjectType, kExtTypeNull)
    INIT_REV_OBJECT_TYPE(int64_t, kExtTypeInt)
    INIT_REV_OBJECT_TYPE(double, kExtTypeFloat)
    INIT_REV_OBJECT_TYPE(int, kExtTypeBool)
    INIT_REV_OBJECT_TYPE(string, kExtTypeString)
    INIT_REV_OBJECT_TYPE(wstring, kExtTypeWideString)
    INIT_REV_OBJECT_TYPE(GenericFunctionPointer, kExtTypeFunctionPointer)
    INIT_REV_OBJECT_TYPE(GenericPointer, kExtTypeObjectPointer)

#undef INIT_REV_OBJECT_TYPE

  constexpr bool _IsStringObject(ObjectType type) {
    return type == kExtTypeString || type == kExtTypeWideString;
  }

  //For future string replacement?
  template <typename _Type>
  struct _CharTypeTrait { using Type = _UnsupportedObjectType; };
  template <>
  struct _CharTypeTrait<string> { using Type = char; };
  template <>
  struct _CharTypeTrait<wstring> { using Type = wchar_t; };

  template <ObjectType _TypeCode>
  struct _CharTypeTraitS { using Type = _UnsupportedObjectType; };
  template <>
  struct _CharTypeTraitS<kExtTypeString> { using Type = char; };
  template <>
  struct _CharTypeTraitS<kExtTypeWideString> { using Type = wchar_t; };

  const unordered_map<string, ObjectType> kExtTypeMatcher = {
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
    ObjectType type;
  };

  using ParameterInformer = const char *(*)(const char *);
  using ObjectTypeFetcher = int(*)(void *, const char *);
  using ErrorInformer = void(*)(void *, const char *);
  using DescriptorFetcher = int(*)(Descriptor *, void *, const char *);
  using ArrayElementFetcher = int(*)(Descriptor *, Descriptor *, size_t);
  using ObjectDumper = int(*)(Descriptor *, void **);
  using DescriptorFetcher = int(*)(Descriptor *, void *, const char *);

  extern "C" struct ExtInterfaces {
    MemoryDisposer disposer;
    ObjectTypeFetcher type_fetcher;
    ErrorInformer error_informer;
    DescriptorFetcher desc_fetcher;
    ArrayElementFetcher arr_elem_fetcher;
    ObjectDumper dumper;
  };

  using ExtensionLoader = int(*)(ExtInterfaces *);

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
