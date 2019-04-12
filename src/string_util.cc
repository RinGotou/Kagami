#include "string_util.h"

namespace kagami {
  bool IsStringFamily(Object &obj) {
    return compare(obj.GetTypeId(), { kTypeIdString,kTypeIdWideString });
  }

  Message CreateStringFromArray(ObjectMap &p) {
    EXPECT_TYPE(p, "src", kTypeIdArray);
    auto &base = p.Cast<ObjectArray>("src");
    shared_ptr<string> dest(make_shared<string>());
    
    for (auto it = base.begin(); it != base.end(); ++it) {
      if (it->GetTypeId() == kTypeIdInt) {
        dest->append(1, static_cast<char>(it->Cast<int64_t>()));
        continue;
      }

      if (it->GetTypeId() != kTypeIdString) {
        continue;
      }

      dest->append(it->Cast<string>());
    }

    return Message().SetObject(Object(dest, kTypeIdString));
  }

  //String
  Message NewString(ObjectMap &p) {
    Object &obj = p["raw_string"];
    Object base;

    EXPECT(IsStringFamily(obj),
      "String constructor can't accept this object.");

    if (obj.GetTypeId() == kTypeIdWideString) {
      wstring wstr = obj.Cast<wstring>();
      string output = ws2s(wstr);

      base.Manage(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag();
    }
    else if (obj.GetTypeId() == kTypeIdString) {
      string copy = obj.Cast<string>();
      base.Manage(make_shared<string>(copy), kTypeIdString)
        .SetConstructorFlag();
    }
    else {
      string output = obj.Cast<string>();

      base.Manage(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag();
    }

    return Message().SetObject(base);
  }

  Message StringCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    string lhs = p[kStrMe].Cast<string>();

    string type_id = rhs.GetTypeId();
    bool result = false;

    if (type_id == kTypeIdString) {
      string rhs_str = rhs.Cast<string>();
      result = (lhs == rhs_str);
    }

    return Message().SetObject(result);
  }

  Message StringToArray(ObjectMap &p) {
    auto &str = p.Cast<string>(kStrMe);
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());
    
    base->reserve(str.size());

    for (auto &unit : str) {
      base->emplace_back(string().append(1, unit));
    }

    return Message().SetObject(Object(base, kTypeIdArray));
  }

  //InStream-new
  Message NewInStream(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    string path = p.Cast<string>("path");

    shared_ptr<wifstream> ifs(make_shared<wifstream>());
    
    ifs->open(path, std::ios::in);

    return Message().SetObject(Object(ifs, kTypeIdInStream));
  }

  Message InStreamGet(ObjectMap &p) {
    wifstream &ifs = p.Cast<wifstream>(kStrMe);

    if (!ifs.good()) {
      return Message(kCodeBadStream, "Invalid instream.", kStateError);
    }

    if (ifs.eof()) {
      return Message("");
    }

    wstring wstr;
    std::getline(ifs, wstr);
    string str = ws2s(wstr);
    if (str.back() == '\n' || str.back() == '\0') str.pop_back();

    return Message().SetObject(str);
  }

  Message InStreamEOF(ObjectMap &p) {
    wifstream &ifs = p.Cast<wifstream>(kStrMe);
    return Message().SetObject(ifs.eof());
  }

  //OutStream
  Message NewOutStream(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    EXPECT_TYPE(p, "mode", kTypeIdString);

    string path = p.Cast<string>("path");
    string mode = p.Cast<string>("mode");

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
    ofstream &ofs = p.Cast<ofstream>(kStrMe);
    bool result = true;

    if (!ofs.good()) {
      result = false;
    }
    else {
      if (p.CheckTypeId("str", kTypeIdString)) {
        string origin = p.Cast<string>("str");
        ofs << origin;
      }
      else {
        result = false;
      }
    }
    
    return Message().SetObject(result);
  }

  //regex
  Message NewRegex(ObjectMap &p) {
    EXPECT_TYPE(p, "pattern", kTypeIdString);

    string pattern_string = p.Cast<string>("pattern");
    shared_ptr<regex> reg = make_shared<regex>(regex(pattern_string));

    return Message().SetObject(Object(reg, kTypeIdRegex));
  }

  Message RegexMatch(ObjectMap &p) {
    EXPECT_TYPE(p, "str", kTypeIdString);

    string str = p.Cast<string>("str");
    auto &pat = p.Cast<regex>(kStrMe);
    bool result = regex_match(str, pat);

    return Message().SetObject(result);
  }

  //wstring
  Message NewWideString(ObjectMap &p) {
    EXPECT_TYPE(p, "raw_string", kTypeIdString);
    Object obj = p["raw_string"];

    string output = obj.Cast<string>();
    wstring wstr = s2ws(output);

    return Message()
      .SetObject(Object(make_shared<wstring>(wstr), kTypeIdWideString));
  }

  Message WideStringCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    wstring lhs = p[kStrMe].Cast<wstring>();
    bool result = false;
    if (rhs.GetTypeId() == kTypeIdWideString) {
      wstring rhs_wstr = rhs.Cast<wstring>();

      result = (lhs == rhs_wstr);
    }

    return Message().SetObject(result);
  }

  //Function
  Message FunctionGetId(ObjectMap &p) {
    auto &interface = p.Cast<Interface>(kStrMe);
    return Message(interface.GetId());
  }

  Message FunctionGetParameters(ObjectMap &p) {
    auto &interface = p.Cast<Interface>(kStrMe);
    shared_ptr<ObjectArray> dest_base = make_shared<ObjectArray>();
    auto origin_vector = interface.GetParameters();

    for (auto it = origin_vector.begin(); it != origin_vector.end(); ++it) {
      dest_base->emplace_back(Object(make_shared<string>(*it), kTypeIdString));
    }

    return Message().SetObject(Object(dest_base, kTypeIdArray));
  }

  Message FunctionCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    auto &lhs = p[kStrMe].Cast<Interface>();

    string type_id = rhs.GetTypeId();
    bool result = false;

    if (type_id == kTypeIdFunction) {
      auto &rhs_interface = rhs.Cast<Interface>();

      result = (lhs == rhs_interface);
    }
    
    return Message().SetObject(result);
  }

  void InitBaseTypes() {
    using management::CreateNewInterface;
    using namespace management::type;

    NewTypeSetup(kTypeIdFunction, SimpleSharedPtrCopy<Interface>)
      .InitMethods(
        {
          Interface(FunctionGetId, "", "id"),
          Interface(FunctionGetParameters, "", "params"),
          Interface(FunctionCompare, kStrRightHandSide, kStrCompare)
        }
    );

    NewTypeSetup(kTypeIdString, SimpleSharedPtrCopy<string>, PlainHasher<string>())
      .InitConstructor(
        Interface(NewString, "raw_string", "string")
      )
      .InitMethods(
        {
          Interface(StringFamilyGetElement<string>, "index", "__at"),
          Interface(StringFamilyPrint<string, std::ostream>, "", "print"),
          Interface(StringFamilySubStr<string>, "start|size", "substr"),
          Interface(GetStringFamilySize<string>, "", "size"),
          Interface(StringFamilyConverting<wstring, string>, "", "to_wide"),
          Interface(StringCompare, kStrRightHandSide, kStrCompare),
          Interface(StringToArray, "","to_array")
        }
    );

    NewTypeSetup(kTypeIdInStream, FakeCopy<wifstream>, PointerHasher())
      .InitConstructor(
        Interface(NewInStream, "path", "instream")
      )
      .InitMethods(
        {
          Interface(InStreamGet, "", "get"),
          Interface(InStreamEOF, "", "eof"),
          Interface(StreamFamilyClose<wifstream>, "", "close")
        }
    );

    NewTypeSetup(kTypeIdOutStream, FakeCopy<ofstream>, PointerHasher())
      .InitConstructor(
        Interface(NewOutStream, "path|mode", "outstream")
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

    NewTypeSetup(kTypeIdRegex, FakeCopy<regex>, PointerHasher())
      .InitConstructor(
        Interface(NewRegex, "pattern", "regex")
      )
      .InitMethods(
        {
          Interface(RegexMatch, "str", "match")
        }
    );

    NewTypeSetup(kTypeIdWideString, SimpleSharedPtrCopy<wstring>, PlainHasher<wstring>())
      .InitConstructor(
        Interface(NewWideString, "raw_string", "wstring")
      )
      .InitMethods(
        {
          Interface(GetStringFamilySize<wstring>,  "", "size"),
          Interface(StringFamilyGetElement<wstring>, "index", "__at"),
          Interface(StringFamilyPrint<wstring, std::wostream>, "", "print"),
          Interface(StringFamilySubStr<wstring>, "start|size", "substr"),
          Interface(StringFamilyConverting<string, wstring>, "", "to_byte"),
          Interface(WideStringCompare, kStrRightHandSide, kStrCompare)
        }
    );

    CreateNewInterface(Interface(DecimalConvert<2>, "str", "bin"));
    CreateNewInterface(Interface(DecimalConvert<8>, "str", "octa"));
    CreateNewInterface(Interface(DecimalConvert<16>, "str", "hex"));
    CreateNewInterface(Interface(CreateStringFromArray, "src", "ar2string"));

    EXPORT_CONSTANT(kTypeIdFunction);
    EXPORT_CONSTANT(kTypeIdString);
    EXPORT_CONSTANT(kTypeIdInStream);
    EXPORT_CONSTANT(kTypeIdOutStream);
    EXPORT_CONSTANT(kTypeIdRegex);
    EXPORT_CONSTANT(kTypeIdWideString);
  }
}