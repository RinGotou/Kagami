#include "base_type.h"

namespace kagami {
  inline bool IsStringFamily(Object &obj) {
    return compare(obj.GetTypeId(), { kTypeIdRawString,kTypeIdString,kTypeIdWideString });
  }

  Message ArrayConstructor(ObjectMap &p) {
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

    if (p.Search("size")) {
      auto size = stoi(p.Cast<string>("size"));
      CONDITION_ASSERT(size > 0, "Illegal array size.");

      Object obj;
      
      p.Search("init_value") ?
        obj.CloneFrom(p["init_value"]) :
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

    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);
    int idx = stoi(p.Cast<string>("index"));
    int size = int(base.size());

    CONDITION_ASSERT(idx < size, "Subscript is out of range.");

    return Message().SetObject(Object().CreateRef(base[idx]));
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrObject];
    return Message(to_string(obj.Cast<ObjectArray>().size()));
  }

  Message ArrayEmpty(ObjectMap &p) {
    return Message(
      util::MakeBoolean(p[kStrObject].Cast<ObjectArray>().empty())
    );
  }

  Message ArrayPush(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);

    base.emplace_back(p["object"]);

    return Message();
  }

  Message ArrayPop(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);

    if (!base.empty()) base.pop_back();

    return Message().SetObject(util::MakeBoolean(base.empty()));
  }
 
  Message ArrayPrint(ObjectMap &p) {
    Message result;
    ObjectMap obj_map;

    auto &base = p.Cast<ObjectArray>(kStrObject);
    auto interface = management::Order("print", kTypeIdNull, -1);

    for (auto &unit : base) {
      obj_map.insert(NamedObject(kStrObject, unit));
      result = interface.Start(obj_map);
      obj_map.clear();
    }

    return result;
  }
  
  //RawString
  Message RawStringGetElement(ObjectMap &p) {
    OBJECT_ASSERT(p, "index", kTypeIdRawString);

    int idx = stoi(p.Cast<string>("index"));

    const auto makeStrToken = [](char target)->string {
      return string().append("'").append(1, target).append("'");
    };

    string data = RealString(p.Cast<string>(kStrObject));
    size_t size = data.size();

    CONDITION_ASSERT(idx < int(size - 1), "Subscript is out of range.");

    return Message(makeStrToken(data.at(idx)));
  }

  Message RawStringGetSize(ObjectMap &p) {
    auto str = RealString(p.Cast<string>(kStrObject));
    return Message(to_string(str.size()));
  }

  Message RawStringPrint(ObjectMap &p) {
    bool doNotWrap = (p.Search("not_wrap"));
    
    auto data = RealString(p.Cast<string>(kStrObject));
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
      wstring wstr = obj.Cast<wstring>();
      string output = ws2s(wstr);

      base.ManageContent(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag();
    }
    else if (obj.GetTypeId() == kTypeIdString) {
      string copy = obj.Cast<string>();
      base.ManageContent(make_shared<string>(copy), kTypeIdString)
        .SetConstructorFlag();
    }
    else {
      string output = RealString(obj.Cast<string>());

      base.ManageContent(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag();
    }

    return Message().SetObject(base);
  }

  //InStream
  Message InStreamConsturctor(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["path"]), 
      "Illegal path.");

    string path = RealString(p.Cast<string>("path"));

    shared_ptr<ifstream> ifs = 
      make_shared<ifstream>(ifstream(path.c_str(), std::ios::in));

    return Message().SetObject(Object(ifs, kTypeIdInStream));
  }

  Message InStreamGet(ObjectMap &p) {
    ifstream &ifs = p.Cast<ifstream>(kStrObject);
    Message msg;

    CUSTOM_ASSERT(ifs.good(), kCodeBadStream, "InStream is not working.");

    if (ifs.eof()) return Message("");

    string str;
    std::getline(ifs, str);

    return Message().SetObject(Object(make_shared<string>(str), kTypeIdString));
  }

  Message InStreamEOF(ObjectMap &p) {
    ifstream &ifs = p.Cast<ifstream>(kStrObject);
    return Message(util::MakeBoolean(ifs.eof()));
  }

  //OutStream
  Message OutStreamConstructor(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["path"]), 
      "Illegal path.");
    CONDITION_ASSERT(IsStringObject(p["mode"]), 
      "Illegal mode option.");

    string path = RealString(p.Cast<string>("path"));
    string mode = RealString(p.Cast<string>("mode"));

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
    ofstream &ofs = p.Cast<ofstream>(kStrObject);
    Message msg = Message(kStrTrue);

    ASSERT_RETURN(ofs.good(), kStrFalse);

    if (p.CheckTypeId("str",kTypeIdRawString)) {
      string output = RealString(p.Cast<string>("str"));
      ofs << output;
    }
    else if (p.CheckTypeId("str",kTypeIdString)) {
      string origin = p.Cast<string>("str");
      ofs << origin;
    }
    else {
      msg = Message(kStrFalse);
    }

    return msg;
  }

  //regex
  Message RegexConstructor(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["pattern"]), 
      "Illegal pattern string.");

    string pattern_string = RealString(p.Cast<string>("pattern"));
    shared_ptr<regex> reg = make_shared<regex>(regex(pattern_string));

    return Message().SetObject(Object(reg, kTypeIdRegex));
  }

  Message RegexMatch(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["str"]), 
      "Illegal target string.");

    string str = RealString(p.Cast<string>("str"));
    auto &pat = p.Cast<regex>(kStrObject);

    return Message(util::MakeBoolean(regex_match(str, pat)));
  }

  //wstring
  Message WideStringContructor(ObjectMap &p) {
    Object obj = p["raw_string"];

    CONDITION_ASSERT(IsStringObject(obj), 
      "String constructor can't accept this object.");

    string output = RealString(obj.Cast<string>());
    wstring wstr = s2ws(output);

    return Message()
      .SetObject(Object(make_shared<wstring>(wstr), kTypeIdWideString));
  }

  //Function
  Message FunctionGetId(ObjectMap &p) {
    auto &interface = p.Cast<Interface>(kStrObject);
    return Message(interface.GetId());
  }

  Message FunctionGetParameters(ObjectMap &p) {
    auto &interface = p.Cast<Interface>(kStrObject);
    shared_ptr<ObjectArray> dest_base = make_shared<ObjectArray>();
    auto origin_vector = interface.GetParameters();

    for (auto it = origin_vector.begin(); it != origin_vector.end(); ++it) {
      dest_base->emplace_back(Object(make_shared<string>(*it), kTypeIdString));
    }

    return Message().SetObject(Object(dest_base, kTypeIdArray));
  }

  bool Generating_AutoSize(Interface &interface, vector<Object> arg_list, ObjectMap &target_map) {
    if (arg_list.size() != interface.GetParameters().size()) return false;
    auto ent_params = interface.GetParameters();
    auto va_arg_head = ent_params.back();

    deque<Object> temp;

    while (arg_list.size() >= ent_params.size() - 1) {
      temp.emplace_back(arg_list.back());
      arg_list.pop_back();
    }

    shared_ptr<ObjectArray> va_base = make_shared<ObjectArray>();

    for (auto it = temp.begin(); it != temp.end(); ++it) {
      va_base->emplace_back(*it);
    }

    temp.clear();
    temp.shrink_to_fit();

    target_map.insert(NamedObject(va_arg_head, Object(va_base, kTypeIdArray)));

    auto it = ent_params.rbegin()++;

    for (; it != ent_params.rend(); ++it) {
      target_map.insert(NamedObject(*it, arg_list.back()));
      arg_list.pop_back();
    }

    if (interface.GetInterfaceType() == kInterfaceTypeMethod) {
      target_map.insert(NamedObject(kStrObject, arg_list.back()));
      arg_list.pop_back();
    }

    return true;
  }

  bool Generating_AutoFill(Interface &interface, vector<Object> arg_list, ObjectMap &target_map) {
    if (arg_list.size() > interface.GetParameters().size()) return false;
    auto ent_params = interface.GetParameters();

    while (ent_params.size() > arg_list.size()) {
      target_map.insert(NamedObject(ent_params.back(), Object()));
      ent_params.pop_back();
    }

    for (auto it = ent_params.rbegin(); it != ent_params.rend(); ++it) {
      target_map.insert(NamedObject(*it, arg_list.back()));
      arg_list.pop_back();
    }

    return true;
  }

  bool Generating(Interface &interface, vector<Object> arg_list, ObjectMap &target_map) {
    if (arg_list.size() != interface.GetParameters().size()) return false;
    auto ent_params = interface.GetParameters();

    for (auto it = ent_params.rbegin(); it != ent_params.rend(); ++it) {
      target_map.insert(NamedObject(*it, arg_list.back()));
      arg_list.pop_back();
    }

    return true;
  }

  Message FunctionCall(ObjectMap &p) {
    auto &interface = p.Cast<Interface>(kStrObject);
    auto &arg_list = p.Cast<ObjectArray>("arg");
    int count = 0;
    ObjectMap target_map;
    bool state;
    
    CALL_ASSERT(interface.Good(), "Bad interface - " + interface.GetId() + ".");

    switch (interface.GetArgumentMode()) {
    case kCodeAutoSize:
      state = Generating_AutoSize(interface, arg_list, target_map);
      break;
    case kCodeAutoFill:
      state = Generating_AutoFill(interface, arg_list, target_map);
    default:
      state = Generating(interface, arg_list, target_map);
      break;
    }

    CONDITION_ASSERT(state, "Argument error.");

    return interface.Start(target_map);
  }

  void InitBaseTypes() {
    using management::type::NewTypeSetup;

    NewTypeSetup(kTypeIdFunction, SimpleSharedPtrCopy<Interface>)
      .InitMethods(
        {
          Interface(FunctionGetId, "", "id"),
          Interface(FunctionCall, "arg", "call", kCodeAutoSize),
          Interface(FunctionGetParameters, "", "params")
        }
    );

    NewTypeSetup(kTypeIdRawString, SimpleSharedPtrCopy<string>)
      .InitMethods(
        {
          Interface(RawStringPrint, "", "__print"),
          Interface(RawStringGetElement, "index", "__at"),
          Interface(RawStringGetSize, "", "size")
        }
    );

    NewTypeSetup(kTypeIdArray, [](shared_ptr<void> source) -> shared_ptr<void> {
        auto &src_base = *static_pointer_cast<ObjectArray>(source);
        shared_ptr<ObjectArray> dest_base = make_shared<ObjectArray>();

        dest_base->reserve(src_base.size());

        for (auto &unit : src_base) {
          dest_base->emplace_back(Object(management::type::GetObjectCopy(unit), unit.GetTypeId()));
        }

        return dest_base;
      })
      .InitConstructor(
        Interface(ArrayConstructor, "size|init_value", "array", kCodeAutoFill)
      )
      .InitMethods(
        {
          Interface(ArrayGetElement, "index", "__at"),
          Interface(ArrayPrint, "", "__print"),
          Interface(ArrayGetSize, "", "size"),
          Interface(ArrayPush, "object", "push"),
          Interface(ArrayPop, "object", "pop"),
          Interface(ArrayEmpty, "", "empty")
        }
    );

    NewTypeSetup(kTypeIdString, SimpleSharedPtrCopy<string>)
      .InitConstructor(
        Interface(StringConstructor, "raw_string", "string")
      )
      .InitMethods(
        {
          Interface(StringFamilyGetElement<string>, "index", "__at"),
          Interface(StringFamilyPrint<string, std::ostream>, "", "__print"),
          Interface(StringFamilySubStr<string>, "start|size", "substr"),
          Interface(GetStringFamilySize<string>, "", "size"),
          Interface(StringFamilyConverting<wstring, string>, "", "to_wide")
        }
    );

    NewTypeSetup(kTypeIdInStream, FakeCopy)
      .InitConstructor(
        Interface(InStreamConsturctor, "path", "instream")
      )
      .InitMethods(
        {
          Interface(InStreamGet, "", "get"),
          Interface(InStreamEOF, "", "eof"),
          Interface(StreamFamilyClose<ifstream>, "", "close")
        }
    );

    NewTypeSetup(kTypeIdOutStream, FakeCopy)
      .InitConstructor(
        Interface(OutStreamConstructor, "path|mode", "outstream")
      )
      .InitMethods(
        {
          Interface(OutStreamWrite, "str", "write"),
          Interface(StreamFamilyState<ofstream>, "", "good"),
          Interface(StreamFamilyClose<ofstream>, "", "close")
        }
    );
    management::CreateConstantObject("kOutstreamModeAppend", Object("'append'"));
    management::CreateConstantObject("kOutstreamModeTruncate", Object("'truncate'"));

    NewTypeSetup(kTypeIdRegex, FakeCopy)
      .InitConstructor(
        Interface(RegexConstructor, "pattern", "regex")
      )
      .InitMethods(
        {
          Interface(RegexMatch, "str", "match")
        }
    );

    NewTypeSetup(kTypeIdWideString, SimpleSharedPtrCopy<wstring>)
      .InitConstructor(
        Interface(WideStringContructor, "raw_string", "wstring")
      )
      .InitMethods(
        {
          Interface(GetStringFamilySize<wstring>,  "", "size"),
          Interface(StringFamilyGetElement<wstring>, "index", "__at"),
          Interface(StringFamilyPrint<wstring, std::wostream>, "", "__print"),
          Interface(StringFamilySubStr<wstring>, "start|size", "substr"),
          Interface(StringFamilyConverting<string, wstring>, "", "to_byte")
        }
    );

    NewTypeSetup(kTypeIdNull, [](shared_ptr<void>) -> shared_ptr<void> { 
      return nullptr; 
    });
  }
}