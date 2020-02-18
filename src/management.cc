#include "management.h"

namespace kagami::management {
///////////////////////////////////////////////////////////////
//Inteface management

  auto &GetFunctionImplCollections() {
    static map<string, FunctionImplCollection> collection_base;
    return collection_base;
  }

  auto &GetFunctionImplCache() {
    static unordered_map<string, FunctionHashMap> cache;
    return cache;
  }

  void BuildFunctionImplCache(string domain) {
    auto &base = GetFunctionImplCache();
    auto &col = GetFunctionImplCollections().at(domain);

    for (auto &unit : col) {
      base[domain].insert(make_pair(unit.first, &unit.second));
    }
  }

  void CreateImpl(FunctionImpl impl, string domain) {
    auto &collection_base = GetFunctionImplCollections();
    auto it = collection_base.find(domain);

    if (it != collection_base.end()) {
      it->second.insert(make_pair(impl.GetId(), impl));
    }
    else {
      collection_base.insert(make_pair(domain, FunctionImplCollection()));
      collection_base[domain].insert(make_pair(impl.GetId(), impl));
    }

    BuildFunctionImplCache(domain);
  }

  FunctionImpl *FindFunction(string id, string domain) {
    auto &cache = GetFunctionImplCache();
    auto it = cache.find(domain);

    if (it != cache.end()) {
      auto dest = it->second.find(id);
      if (dest != it->second.end()) {
        return dest->second;
      }
    }

    return nullptr;
  }
  ////////////////////////////////////////////////////////////////

  ////////////////////////////////////////////////////////////////
  //Constant object management

  auto &GetConstantBase() {
    static ObjectContainer base;
    return base;
  }

  Object *CreateConstantObject(string id, Object &object) {
    ObjectContainer &base = GetConstantBase();

    if (base.Find(id) != nullptr) return nullptr;

    base.Add(id, object);
    auto result = base.Find(id);
    return result;
  }

  Object *CreateConstantObject(string id, Object && object) {
    return CreateConstantObject(id, object);
  }

  Object GetConstantObject(string id) {
    ObjectContainer &base = GetConstantBase();
    auto ptr = base.Find(id);

    if (ptr != nullptr) {
      Object obj = type::CreateObjectCopy(*ptr);
      return obj;
    }

    return Object();
  }

  /////////////////////////////////////////////////////////////
}

namespace kagami::management::type {
  auto &GetObjectTraitsCollection() {
    static unordered_map<string, ObjectTraits> base;
    return base;
  }

  vector<string> GetMethods(string id) {
    vector<string> result;
    const auto it = GetObjectTraitsCollection().find(id);

    if (it != GetObjectTraitsCollection().end()) {
      result = it->second.GetMethods();
    }
    return result;
  }

  bool CheckMethod(string func_id, string domain) {
    bool result = false;
    const auto it = GetObjectTraitsCollection().find(domain);

    if (it != GetObjectTraitsCollection().end()) {
      result = find_in_vector(func_id, it->second.GetMethods());
    }

    return result;
  }

  size_t GetHash(Object &obj) {
    auto &base = GetObjectTraitsCollection();
    const auto it = base.find(obj.GetTypeId());
    auto hasher = it->second.GetHasher();
    return hasher(obj.Get());
  }

  bool IsHashable(Object &obj) {
    bool result = false;
    auto &base = GetObjectTraitsCollection();
    const auto it = base.find(obj.GetTypeId());

    if (it != base.end()) {
      result = (it->second.GetHasher() != nullptr);
    }

    return result;
  }

  bool IsCopyable(Object &obj) {
    bool result = false;
    auto &base = GetObjectTraitsCollection();
    const auto it = base.find(obj.GetTypeId());

    if (it != base.end()) {
      result = (it->second.GetDeliveringImpl() != ShallowDelivery);
    }

    return result;
  }

  void CreateObjectTraits(string id, ObjectTraits temp) {
    GetObjectTraitsCollection().insert(pair<string, ObjectTraits>(id, temp));
  }

  //TODO:External/Delegator object processing
  Object CreateObjectCopy(Object &object) {
    if (object.GetDeliveringFlag()) {
      return object;
    }

    Object result;
    const auto it = GetObjectTraitsCollection().find(object.GetTypeId());

    if (object.GetMode() == kObjectExternal) {
      //TODO:Implementations (After finishing callback facility for types)
    }
    else if (object.GetMode() == kObjectDelegator) {
      result = object;
    }
    else if (object.IsSubContainer() && object.GetTypeId() != kTypeIdStruct) {
      auto &instance_src = object.Cast<ObjectStruct>();
      auto &base = instance_src.GetContent();
      auto managed_instance = make_shared<ObjectStruct>();
      for (auto &unit : base) {
        auto copy = CreateObjectCopy(unit.second);
        managed_instance->Add(unit.first, copy);
      }
      result = Object(managed_instance, object.GetTypeId());
      result.SetContainerFlag();
    }
    else if (it != GetObjectTraitsCollection().end()) {
      auto deliver = it->second.GetDeliveringImpl();
      result.PackContent(deliver(object.Get()), object.GetTypeId());
    }

    return result;
  }

  bool CheckBehavior(Object obj, string method_str) {
    auto obj_methods = GetMethods(obj.GetTypeId());
    auto sample = BuildStringVector(method_str);
    bool result = true;

    for (auto &unit : sample) {
      if (!find_in_vector(unit, obj_methods)) {
        result = false;
        break;
      }
    }

    return result;
  }

  bool CompareObjects(Object &lhs, Object &rhs) {
    if (lhs.GetTypeId() != rhs.GetTypeId()) return false;
    auto &collection = GetObjectTraitsCollection();
    const auto it = collection.find(lhs.GetTypeId());
    bool value = false;

    if (it != collection.end()) {
      auto comparator = it->second.GetComparator();
      if (comparator != nullptr) value = comparator(lhs, rhs);
      else value = (lhs.Get() == rhs.Get());
    }

    return value;
  }

  ObjectTraitsSetup &ObjectTraitsSetup::InitMethods(initializer_list<FunctionImpl> && rhs) {
    impl_ = rhs;
    string method_list("");

    for (auto &unit : impl_) {
      method_list.append(unit.GetId()).append("|");
    }

    if (method_list != "") {
      methods_ = method_list.substr(0, method_list.size() - 1);
    }

    return *this;
  }

  ObjectTraitsSetup::~ObjectTraitsSetup() {
    CreateObjectTraits(type_id_, ObjectTraits(delivering_impl_, methods_, hasher_, comparator_));
    CreateImpl(delivering_);
    for (auto &unit : impl_) {
      CreateImpl(unit, type_id_);
    }
  }
}

namespace kagami::management::script {
  auto &GetScriptStorage() {
    static ScriptStorage storage;
    return storage;
  }

  VMCode *FindScriptByPath(string path) {
    VMCode result = nullptr;
    auto &storage = GetScriptStorage();
    auto it = storage.find(path);
    
    if (it != storage.end()) return &(it->second);

    return nullptr;
  }

  VMCode &AppendScript(string path, VMCode &code) {
    auto &storage = GetScriptStorage();
    ScriptStorage::iterator it;
    
    it = storage.find(path);

    if (it != storage.end()) return it->second;

    VMCode script;
    
    storage.insert(make_pair(path, code));
    it = storage.find(path);

    return it->second;
  }

  VMCode &AppendBlankScript(string path) {
    auto &storage = GetScriptStorage();
    ScriptStorage::iterator it;

    it = storage.find(path);

    if (it != storage.end()) return it->second;

    VMCode script;


    storage.insert(make_pair(path, VMCode()));
    it = storage.find(path);

    return it->second;
  }
}

namespace kagami::management::extension {
  template <ObjectType _TypeCode>
  struct _Deleter {
    _Deleter(void *ptr) {
      if constexpr (_IsStringObject(_TypeCode)) {
        delete[] (typename _CharTypeTraitS<_TypeCode>::Type *)ptr;
      }
      else {
        delete (typename _ObjectTypeTrait<_TypeCode>::Type *)ptr;
      }
    }
  };

  void DisposeMemoryUnit(void *ptr, int type) {
    switch (type) {
    case kExtTypeInt:_Deleter<kExtTypeInt>((void *)ptr); break;
    case kExtTypeFloat:_Deleter<kExtTypeFloat>((void *)ptr); break;
    case kExtTypeBool:_Deleter<kExtTypeBool>((void *)ptr); break;
    case kExtTypeFunctionPointer:
      _Deleter<kExtTypeFunctionPointer>((void *)ptr); break;
    case kExtTypeObjectPointer:
      _Deleter<kExtTypeObjectPointer>((void *)ptr); break;
    default:break;
    }
  }

  inline ObjectType MatchExtType(string type_id) {
    auto result = kExtUnsupported;
    auto it = kExtTypeMatcher.find(type_id);
    if (it != kExtTypeMatcher.end()) result = it->second;
    return result;
  }

  int FetchDescriptor(Descriptor *descriptor, void *obj_map, const char *id) {
    auto *source = static_cast<ObjectMap *>(obj_map);
    auto it = source->find(string(id));
    if (it == source->end()) return 0;
    *descriptor = Descriptor{ &it->second, MatchExtType(it->second.GetTypeId()) };
    return 1;
  }

  //for variable argument pattern
  // -1 : type mismatch
  // 0 : index out of range
  int FetchArrayElementDescriptor(Descriptor *arr_desc, Descriptor *dest, size_t index) {
    if (arr_desc->type != kExtTypeArray) return -1;
    auto &arr_obj = *static_cast<Object *>(arr_desc->ptr);
    auto &arr = arr_obj.Cast<ObjectArray>();
    if (index >= arr.size()) return 0;
    void *elem_ptr = &arr[index];
    *dest = Descriptor{ elem_ptr, MatchExtType(arr[index].GetTypeId()) };
    return 1;
  }

  size_t GetArrayObjectCapacity(Descriptor desc) {
    if (desc.type != kExtTypeArray) return 0;
    auto &arr = *static_cast<Object *>(desc.ptr);
    return arr.Cast<ObjectArray>().size();
  }

  template <typename _Type>
  void _DumpObjectA(Object &obj, void **dest) {
    auto &value = obj.Cast<_Type>();
    *dest = new _Type(value);
  }

  template <typename _CharType>
  using _StrCpyFunc = _CharType * (*)(_CharType *, const _CharType *);

  template <typename _CharType>
  struct _StrCpySelector{ 
    _StrCpyFunc<_CharType> func;
  };

  template <>
  struct _StrCpySelector<char> {
    _StrCpyFunc<char> func;
    _StrCpySelector() : func(std::strcpy) {}
  };

  template <>
  struct _StrCpySelector<wchar_t> {
    _StrCpyFunc<wchar_t> func;
    _StrCpySelector() : func(std::wcscpy) {}
  };

  template <typename _CharType, typename _StringType>
  void _DumpObjectS(Object &obj, void **dest) {
    auto &value = obj.Cast<_StringType>();
    *dest = new _CharType[value.size() + 1];
    _StrCpySelector<_CharType>().func((_CharType *)*dest, value.data());
  }

  //Old fetching facilities will be deprecated in the future
  // 0 : type mismatch
  // -1 : unsupported
  int DumpObjectFromDescriptor(Descriptor *descriptor, void **dest) {
    auto &obj = *static_cast<ObjectPointer>(descriptor->ptr);

    if (descriptor->type == kExtUnsupported) return -1;
    if (descriptor->type != MatchExtType(obj.GetTypeId())) return 0;

    auto is_string = [](auto type) -> bool {
      return type == kExtTypeString || type == kExtTypeWideString; };

    if (is_string(descriptor->type)) {
      switch (descriptor->type) {
      case kExtTypeString:_DumpObjectS<char, string>(obj, dest); break;
      case kExtTypeWideString:_DumpObjectS<wchar_t, wstring>(obj, dest); break;
      default:break;
      }
    }
    else {
      switch (descriptor->type) {
      case kExtTypeInt:_DumpObjectA<int64_t>(obj, dest); break;
      case kExtTypeFloat:_DumpObjectA<double>(obj, dest); break;
      case kExtTypeBool:_DumpObjectA<int>(obj, dest); break;
      case kExtTypeFunctionPointer:
        _DumpObjectA<GenericFunctionPointer>(obj, dest);break;
      case kExtTypeObjectPointer:_DumpObjectA<GenericPointer>(obj, dest); break;
      default:break;
      }
    }

    return 1;
  }

  int FetchCustomTypes(void **target, void *obj_map, const char *id) {
    auto *source = static_cast<ObjectMap *>(obj_map);
    auto it = source->find(string(id));
    if (it == source->end()) return 0;
    if (it->second.GetMode() != kObjectExternal) return -1;
    *target = it->second.GetExternalPointer();
    return 1;
  }

  int FetchObjectType(void *obj_map, const char *id) {
    auto &p = *static_cast<ObjectMap *>(obj_map);
    auto it = p.find(string(id));
    int result = kExtTypeNull;

    if (it != p.end()) {
      auto type = it->second.GetTypeId();
      if (type == kTypeIdInt) result = kExtTypeInt;
      else if (type == kTypeIdFloat) result = kExtTypeFloat;
      else if (type == kTypeIdBool) result = kExtTypeBool;
      else if (type == kTypeIdString) result = kExtTypeString;
      else if (type == kTypeIdWideString) result = kExtTypeWideString;
      //TODO:
    }

    return result;
  }
}

namespace kagami::management::runtime {
  static string binary_name;
  static string binary_path;
  static string script_work_dir;
  static fs::path script_absolute_path;

  void InformBinaryPathAndName(string info) {
    fs::path processed_path(info);
    binary_name = processed_path.filename().string();
    binary_path = processed_path.parent_path().string();
  }

  string GetBinaryPath() { return binary_path; }
  string GetBinaryName() { return binary_name; }

  string GetWorkingDirectory() {
#ifdef _WIN32
    //using recommended implementation from Microsoft's document
    auto *buffer = _getcwd(nullptr, 0);
#else
    auto *buffer = getcwd(nullptr, 0);
#endif

    if (buffer == nullptr) return "";

    string result(buffer);
    free(buffer);
    return result;
  }

  bool SetWorkingDirectory(string dir) {
#ifdef _WIN32
    int ret = _chdir(dir.data());
#else
    int ret = chdir(dir.data());
#endif

    return ret == 0;
  }

  void InformScriptPath(string path) {
    script_absolute_path = fs::absolute(fs::path(path)).parent_path();
  }

  string GetScriptAbsolutePath() {
    return script_absolute_path.string();
  }
}