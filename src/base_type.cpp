#include "base_type.h"

namespace kagami {
  //Array
  shared_ptr<void> CreateArrayCopy(shared_ptr<void> source) {
    auto &src_base = *static_pointer_cast<ArrayBase>(source);
    shared_ptr<ArrayBase> dest_base = make_shared<ArrayBase>();

    dest_base->reserve(src_base.size());

    for (auto &unit : src_base) {
      dest_base->emplace_back(Object(management::type::GetObjectCopy(unit), unit.GetTypeId()));
    }

    return dest_base;
  }

  inline bool IsStringFamily(Object &obj) {
    return compare(obj.GetTypeId(), { kTypeIdRawString,kTypeIdString,kTypeIdWideString });
  }

  Message ArrayConstructor(ObjectMap &p) {
    shared_ptr<ArrayBase> base(make_shared<ArrayBase>());

    if (p.Search("size")) {
      auto size = stoi(p.Get<string>("size"));
      CONDITION_ASSERT(size > 0, "Illegal array size.");

      Object obj;
      
      p.Search("init_value") ?
        obj.Copy(p["init_value"]) :
        obj = Object();

      base->reserve(size);
      auto type_id = obj.GetTypeId();

      for (auto count = 0; count < size; count++) {
        base->emplace_back(Object(management::type::GetObjectCopy(obj), type_id));
      }
    }

    return Message().SetObject(Object(base, kTypeIdArray).SetConstructorFlag());
  }

  Message ArrayGetElement(ObjectMap &p) {
    OBJECT_ASSERT(p, "index", kTypeIdRawString);

    ArrayBase &base = p.Get<ArrayBase>(kStrObject);
    int idx = stoi(p.Get<string>("index"));
    int size = int(base.size());

    CONDITION_ASSERT(idx < size, "Subscript is out of range.");

    return Message().SetObject(Object().Ref(base[idx]));
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrObject];
    return Message(to_string(GetObjectStuff<ArrayBase>(obj).size()));
  }

  Message ArrayEmpty(ObjectMap &p) {
    return Message(
      GetObjectStuff<ArrayBase>(p[kStrObject]).empty() ?
      kStrTrue : kStrFalse
    );
  }

  Message ArrayPush(ObjectMap &p) {
    ArrayBase &base = p.Get<ArrayBase>(kStrObject);

    base.emplace_back(p["object"]);

    return Message();
  }

  Message ArrayPop(ObjectMap &p) {
    ArrayBase &base = p.Get<ArrayBase>(kStrObject);

    if (!base.empty()) base.pop_back();

    return Message().SetObject(
      base.empty() ? kStrTrue : kStrFalse
    );
  }
 
  Message ArrayPrint(ObjectMap &p) {
    Message result;
    ObjectMap obj_map;

    auto &base = p.Get<ArrayBase>(kStrObject);
    auto interface = management::Order("print", kTypeIdNull, -1);

    for (auto &unit : base) {
      obj_map.Input(kStrObject, unit);
      result = interface.Start(obj_map);
      obj_map.clear();
    }

    return result;
  }
  
  //RawString
  Message RawStringGetElement(ObjectMap &p) {
    OBJECT_ASSERT(p, "index", kTypeIdRawString);

    int idx = stoi(p.Get<string>("index"));

    const auto makeStrToken = [](char target)->string {
      return string().append("'").append(1, target).append("'");
    };

    string data = RealString(p.Get<string>(kStrObject));
    size_t size = data.size();

    CONDITION_ASSERT(idx < int(size - 1), "Subscript is out of range.");

    return Message(makeStrToken(data.at(idx)));
  }

  Message RawStringGetSize(ObjectMap &p) {
    auto str = RealString(p.Get<string>(kStrObject));
    return Message(to_string(str.size()));
  }

  Message RawStringPrint(ObjectMap &p) {
    bool doNotWrap = (p.Search("not_wrap"));
    
    auto data = RealString(p.Get<string>(kStrObject));
    std::cout << data;
    if (!doNotWrap) std::cout << std::endl;
    
    return Message();
  }

  //String
  Message StringConstructor(ObjectMap &p) {
    Object &obj = p["raw_string"];
    Object base;

    CONDITION_ASSERT(IsStringFamily(obj),
      "String constructor can't accept this object.");

    if (obj.GetTypeId() == kTypeIdWideString) {
      wstring wstr = GetObjectStuff<wstring>(obj);
      string output = ws2s(wstr);

      base.Set(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag();
    }
    else if (obj.GetTypeId() == kTypeIdString) {
      base.Set(obj.Get(), kTypeIdString)
        .SetConstructorFlag();
    }
    else {
      string output = RealString(GetObjectStuff<string>(obj));

      base.Set(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag();
    }

    return Message().SetObject(base);
  }

  //InStream
  Message InStreamConsturctor(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["path"]), 
      "Illegal path.");

    string path = RealString(p.Get<string>("path"));

    shared_ptr<ifstream> ifs = 
      make_shared<ifstream>(ifstream(path.c_str(), std::ios::in));

    return Message().SetObject(Object(ifs, kTypeIdInStream));
  }

  Message InStreamGet(ObjectMap &p) {
    ifstream &ifs = p.Get<ifstream>(kStrObject);
    Message msg;

    CUSTOM_ASSERT(ifs.good(), kCodeBadStream, "InStream is not working.");

    if (ifs.eof()) return Message("");

    string str;
    std::getline(ifs, str);

    return Message().SetObject(Object(make_shared<string>(str), kTypeIdString));
  }

  Message InStreamEOF(ObjectMap &p) {
    ifstream &ifs = p.Get<ifstream>(kStrObject);
    return Message(ifs.eof() ? kStrTrue : kStrFalse);
  }

  //OutStream
  Message OutStreamConstructor(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["path"]), 
      "Illegal path.");
    CONDITION_ASSERT(IsStringObject(p["mode"]), 
      "Illegal mode option.");

    string path = RealString(p.Get<string>("path"));
    string mode = RealString(p.Get<string>("mode"));

    shared_ptr<ofstream> ofs;
    bool append = (mode == "append");
    bool truncate = (mode == "truncate");

    if (!append && truncate) {
      ofs = make_shared<ofstream>(ofstream(path.c_str(), 
        std::ios::out | std::ios::trunc));
    }
    else if (append && !truncate) {
      ofs = make_shared<ofstream>(ofstream(path.c_str(),
        std::ios::out | std::ios::app));
    }

    return Message().SetObject(Object(ofs, kTypeIdOutStream));
  }

  Message OutStreamWrite(ObjectMap &p) {
    ofstream &ofs = p.Get<ofstream>(kStrObject);
    Message msg = Message(kStrTrue);

    ASSERT_RETURN(ofs.good(), kStrFalse);

    if (p.CheckTypeId("str",kTypeIdRawString)) {
      string output = RealString(p.Get<string>("str"));
      ofs << output;
    }
    else if (p.CheckTypeId("str",kTypeIdString)) {
      string origin = p.Get<string>("str");
      ofs << origin;
    }
    else {
      msg = Message(kStrFalse);
    }

    return msg;
  }

  //regex
  Message RegexConstructor(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["regex"]), 
      "Illegal pattern string.");

    string pattern_string = RealString(p.Get<string>("regex"));
    shared_ptr<regex> reg = make_shared<regex>(regex(pattern_string));

    return Message().SetObject(Object(reg, kTypeIdRegex));
  }

  Message RegexMatch(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["str"]), 
      "Illegal target string.");

    string str = RealString(p.Get<string>("str"));
    auto &pat = p.Get<regex>(kStrObject);

    return Message(regex_match(str, pat) ? kStrTrue : kStrFalse);
  }

  //wstring
  Message WideStringContructor(ObjectMap &p) {
    Object obj = p["raw_string"];

    CONDITION_ASSERT(IsStringObject(obj), 
      "String constructor can't accept this object.");

    string output = RealString(GetObjectStuff<string>(obj));
    wstring wstr = s2ws(output);

    return Message()
      .SetObject(Object(make_shared<wstring>(wstr), kTypeIdWideString));
  }

  //Function
  Message FunctionGetId(ObjectMap &p) {
    auto &interface = p.Get<Interface>(kStrObject);
    return Message(interface.GetId());
  }

  Message FunctionGetParameters(ObjectMap &p) {
    auto &interface = p.Get<Interface>(kStrObject);
    shared_ptr<ArrayBase> dest_base = make_shared<ArrayBase>();
    auto origin_vector = interface.GetParameters();

    for (auto it = origin_vector.begin(); it != origin_vector.end(); ++it) {
      dest_base->emplace_back(Object(make_shared<string>(*it), kTypeIdString));
    }

    return Message().SetObject(Object(dest_base, kTypeIdArray));
  }

  bool AssemblingForAutosized(Interface &interface, ObjectMap &p, ObjectMap &target_map, int size) {
    auto ent_params = interface.GetParameters();
    auto va_arg_head = ent_params.back();
    int idx = 0;
    int va_arg_size = 0;
    int count = 0;
    

    while (idx < int(ent_params.size() - 1)) {
      target_map.Input(ent_params[idx], p["arg" + to_string(idx)]);
      idx += 1;
    }

    interface.GetInterfaceType() == kInterfaceTypeMethod ?
      va_arg_size = size - 1 :
      va_arg_size = size;

    while (idx < va_arg_size) {
      target_map.Input(va_arg_head + to_string(count), p["arg" + to_string(idx)]);
      count += 1;
      idx += 1;
    }

    target_map.Input(kStrVaSize, Object(to_string(count)));
    if (interface.GetInterfaceType() == kInterfaceTypeMethod) target_map.Input(kStrObject, p["arg" + to_string(size - 1)]);
    return true;
  }

  bool AssemblingForAutoFilling(Interface &interface, ObjectMap &p, ObjectMap &target_map, int size) {
    auto ent_params = interface.GetParameters();
    int idx = 0;
    auto is_method = (interface.GetInterfaceType() == kInterfaceTypeMethod);

    while (idx < ent_params.size()) {
      if (idx >= size) break;
      if (idx >= size - 1 && is_method) break;
      target_map.Input(ent_params[idx], p["arg" + to_string(idx)]);
      idx += 1;
    }

    if (is_method) target_map.Input(kStrObject, p["arg" + to_string(size - 1)]);
    return true;
  }

  bool AssemblingForNormal(Interface &interface, ObjectMap &p, ObjectMap &target_map, int size) {
    auto ent_params = interface.GetParameters();
    int idx = 0;
    auto is_method = (interface.GetInterfaceType() == kInterfaceTypeMethod);
    bool state = true;

    while (idx < ent_params.size()) {
      if (idx >= size || (idx >= size - 1 && is_method)) {
        state = false;
        break;
      }

      target_map.Input(ent_params[idx], p["arg" + to_string(idx)]);
      idx += 1;
    }

    if (is_method && state) 
      target_map.Input(kStrObject, p["arg" + to_string(size - 1)]);

    return state;
  }

  Message FunctionCall(ObjectMap &p) {
    auto &interface = p.Get<Interface>(kStrObject);
    int size = stoi(p.Get<string>(kStrVaSize));
    int count = 0;
    ObjectMap target_map;
    bool state;
    
    CALL_ASSERT(interface.Good(), "Bad interface - " + interface.GetId() + ".");

    switch (interface.GetArgumentMode()) {
    case kCodeAutoSize:
      state = AssemblingForAutosized(interface, p, target_map, size);
      break;
    case kCodeAutoFill:
      state = AssemblingForAutoFilling(interface, p, target_map, size);
    default:
      state = AssemblingForNormal(interface, p, target_map, size);
      break;
    }

    CONDITION_ASSERT(state, "Argument error.");

    return interface.Start(target_map);
  }

  void InitPlanners() {
    using management::type::NewType;
    using management::CreateInterface;

    NewType(kTypeIdFunction, ObjectPolicy(SimpleSharedPtrCopy<Interface>, kFunctionMethods));
    CreateInterface({
      Interface(FunctionGetId, "", "id", kTypeIdFunction),
      Interface(FunctionCall, "arg", "call", kTypeIdFunction, kCodeAutoSize),
      Interface(FunctionGetParameters, "", "params", kTypeIdFunction)
      });


    NewType(kTypeIdRawString, ObjectPolicy(SimpleSharedPtrCopy<string>, kRawStringMethods));
    CreateInterface({
      Interface(RawStringPrint, "", "__print", kTypeIdRawString),
      Interface(RawStringGetElement, "index", "__at", kTypeIdRawString),
      Interface(RawStringGetSize, "", "size", kTypeIdRawString)
      });

    NewType(kTypeIdArray, ObjectPolicy(CreateArrayCopy, kArrayMethods));
    CreateInterface({
      Interface(ArrayConstructor, "size|init_value", "array", kCodeAutoFill),
      Interface(ArrayGetElement, "index", "__at", kTypeIdArray),
      Interface(ArrayPrint, "", "__print", kTypeIdArray),
      Interface(ArrayGetSize, "", "size", kTypeIdArray),
      Interface(ArrayPush, "push", "object", kTypeIdArray),
      Interface(ArrayPop, "pop", "object", kTypeIdArray),
      Interface(ArrayEmpty, "empty", "", kTypeIdArray)
      });

    NewType(kTypeIdString, ObjectPolicy(SimpleSharedPtrCopy<string>, kStringMethods));
    CreateInterface({
      Interface(StringConstructor, "raw_string", "string"),
      Interface(StringFamilyGetElement<string>, "index", "__at", kTypeIdString),
      Interface(StringFamilyPrint<string, std::ostream>, "", "__print", kTypeIdString),
      Interface(StringFamilySubStr<string>, "start|size", "substr", kTypeIdString),
      Interface(GetStringFamilySize<string>, "", "size", kTypeIdString),
      Interface(StringFamilyConverting<wstring, string>, "", "to_wide",  kTypeIdString)
      });

    NewType(kTypeIdInStream, ObjectPolicy(FakeCopy, kInStreamMethods));
    CreateInterface({
      Interface(InStreamConsturctor, "path", "instream"),
      Interface(InStreamGet, "", "get", kTypeIdInStream),
      Interface(InStreamEOF, "", "eof", kTypeIdInStream),
      Interface(StreamFamilyClose<ifstream>, "", "close", kTypeIdInStream)
      });

    NewType(kTypeIdOutStream, ObjectPolicy(FakeCopy, kOutStreamMethods));
    CreateInterface({
      Interface(OutStreamConstructor, "path|mode", "outstream"),
      Interface(OutStreamWrite, "str", "write", kTypeIdOutStream),
      Interface(StreamFamilyState<ofstream>, "", "good", kTypeIdOutStream),
      Interface(StreamFamilyClose<ofstream>, "", "close", kTypeIdOutStream)
      });
    management::CreateConstantObject("kOutstreamModeAppend", Object("'append'"));
    management::CreateConstantObject("kOutstreamModeTruncate", Object("'truncate'"));

    NewType(kTypeIdRegex, ObjectPolicy(FakeCopy, kTypeIdRegex));
    CreateInterface({
      Interface(RegexConstructor, "regex", "regex"),
      Interface(RegexMatch, "str", "match", kTypeIdRegex)
      });

    NewType(kTypeIdWideString, ObjectPolicy(SimpleSharedPtrCopy<wstring>, kWideStringMethods));
    CreateInterface({
      Interface(WideStringContructor, "raw_string", "wstring"),
      Interface(GetStringFamilySize<wstring>,  "", "size", kTypeIdWideString),
      Interface(StringFamilyGetElement<wstring>, "index", "__at", kTypeIdWideString),
      Interface(StringFamilyPrint<wstring, std::wostream>, "", "__print", kTypeIdWideString),
      Interface(StringFamilySubStr<wstring>, "start|size", "substr", kTypeIdWideString),
      Interface(StringFamilyConverting<string, wstring>, "", "to_byte", kTypeIdWideString)
      });

    NewType(kTypeIdNull, ObjectPolicy(NullCopy, ""));
  }
}