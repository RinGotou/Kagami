#include "base_type.h"

namespace kagami {
  bool IsStringFamily(Object &obj) {
    return compare(obj.GetTypeId(), { kTypeIdRawString,kTypeIdString,kTypeIdWideString });
  }
  
  //RawString
  Message RawStringGetElement(ObjectMap &p) {
    EXPECT_TYPE(p, "index", kTypeIdRawString);

    size_t idx = stol(p.Cast<string>("index"));

    const auto makeStrToken = [](char target)->string {
      return string().append("'").append(1, target).append("'");
    };

    string data = ParseRawString(p.Cast<string>(kStrObject));
    size_t size = data.size();

    EXPECT(idx < size - 1, "Subscript is out of range.");

    return Message(makeStrToken(data.at(idx)));
  }

  Message RawStringGetSize(ObjectMap &p) {
    auto str = ParseRawString(p.Cast<string>(kStrObject));
    return Message(to_string(str.size()));
  }

  Message RawStringPrint(ObjectMap &p) {
    auto str = ParseRawString(p.Cast<string>(kStrObject));
    std::cout << str;
    
    return Message();
  }

  //String
  Message StringConstructor(ObjectMap &p) {
    Object &obj = p["raw_string"];
    Object base;

    EXPECT(IsStringFamily(obj),
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
      string output = ParseRawString(obj.Cast<string>());

      base.ManageContent(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag();
    }

    return Message().SetObject(base);
  }

  Message StringCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    string lhs = p[kStrObject].Cast<string>();

    string type_id = rhs.GetTypeId();
    bool result = false;

    if (type_id == kTypeIdRawString) {
      string rhs_str = ParseRawString(rhs.Cast<string>());
      result = (lhs == rhs_str);
    }
    else if (type_id == kTypeIdString) {
      string rhs_str = rhs.Cast<string>();
      result = (lhs == rhs_str);
    }

    return Message(util::MakeBoolean(result));
  }

  //InStream
  Message InStreamConsturctor(ObjectMap &p) {
    EXPECT(IsStringObject(p["path"]), 
      "Illegal path.");

    string path = ParseRawString(p.Cast<string>("path"));

    shared_ptr<ifstream> ifs = 
      make_shared<ifstream>(ifstream(path.c_str(), std::ios::in));

    return Message().SetObject(Object(ifs, kTypeIdInStream));
  }

  Message InStreamGet(ObjectMap &p) {
    ifstream &ifs = p.Cast<ifstream>(kStrObject);
    Message msg;

    if (!ifs.good()) {
      return Message(kCodeBadStream, "Invalid instream.", kStateError);
    }

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
    EXPECT(IsStringObject(p["path"]), 
      "Illegal path.");
    EXPECT(IsStringObject(p["mode"]), 
      "Illegal mode option.");

    string path = ParseRawString(p.Cast<string>("path"));
    string mode = ParseRawString(p.Cast<string>("mode"));

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

    if (!ofs.good()) {
      return Message(kStrFalse);
    }

    if (p.CheckTypeId("str",kTypeIdRawString)) {
      string output = ParseRawString(p.Cast<string>("str"));
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
    EXPECT(IsStringObject(p["pattern"]), 
      "Illegal pattern string.");

    string pattern_string = ParseRawString(p.Cast<string>("pattern"));
    shared_ptr<regex> reg = make_shared<regex>(regex(pattern_string));

    return Message().SetObject(Object(reg, kTypeIdRegex));
  }

  Message RegexMatch(ObjectMap &p) {
    EXPECT(IsStringObject(p["str"]), 
      "Illegal target string.");

    string str = ParseRawString(p.Cast<string>("str"));
    auto &pat = p.Cast<regex>(kStrObject);

    return Message(util::MakeBoolean(regex_match(str, pat)));
  }

  //wstring
  Message WideStringContructor(ObjectMap &p) {
    Object obj = p["raw_string"];

    EXPECT(IsStringObject(obj), 
      "String constructor can't accept this object.");

    string output = ParseRawString(obj.Cast<string>());
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

  bool Generating_AutoSize(Interface &interface, ObjectArray arg_list, ObjectMap &target_map) {
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

  bool Generating_AutoFill(Interface &interface, ObjectArray arg_list, ObjectMap &target_map) {
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

  bool Generating(Interface &interface, ObjectArray arg_list, ObjectMap &target_map) {
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
    ObjectMap target_map;
    bool state;
    
    if (!interface.Good()) {
      return Message(kCodeIllegalCall, "Bad interface - " + interface.GetId() + ".", kStateError);
    }

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

    EXPECT(state, "Argument error.");

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
          Interface(StringFamilyConverting<wstring, string>, "", "to_wide"),
          Interface(StringCompare, kStrRightHandSide, kStrCompare)
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