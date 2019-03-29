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
      if (it->GetTypeId() != kTypeIdString) {
        continue;
      }

      dest->append(it->Cast<string>());
    }

    return Message().SetObject(Object(dest, kTypeIdString));
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
      string output = obj.Cast<string>();

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

    if (type_id == kTypeIdString) {
      string rhs_str = rhs.Cast<string>();
      result = (lhs == rhs_str);
    }

    return Message().SetObject(result);
  }

  Message StringToArray(ObjectMap &p) {
    auto &str = p.Cast<string>(kStrObject);
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());
    
    base->reserve(str.size());

    for (auto &unit : str) {
      base->emplace_back(string().append(1, unit));
    }

    return Message().SetObject(Object(base, kTypeIdArray));
  }

  //InStream
  Message InStreamConsturctor(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);

    string path = p.Cast<string>("path");

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
    return Message().SetObject(ifs.eof());
  }

  //OutStream
  Message OutStreamConstructor(ObjectMap &p) {
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
    ofstream &ofs = p.Cast<ofstream>(kStrObject);
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
  Message RegexConstructor(ObjectMap &p) {
    EXPECT_TYPE(p, "pattern", kTypeIdString);

    string pattern_string = p.Cast<string>("pattern");
    shared_ptr<regex> reg = make_shared<regex>(regex(pattern_string));

    return Message().SetObject(Object(reg, kTypeIdRegex));
  }

  Message RegexMatch(ObjectMap &p) {
    EXPECT_TYPE(p, "str", kTypeIdString);

    string str = p.Cast<string>("str");
    auto &pat = p.Cast<regex>(kStrObject);
    bool result = regex_match(str, pat);


    return Message().SetObject(result);
  }

  //wstring
  Message WideStringContructor(ObjectMap &p) {
    EXPECT_TYPE(p, "raw_string", kTypeIdString);
    Object obj = p["raw_string"];

    string output = obj.Cast<string>();
    wstring wstr = s2ws(output);

    return Message()
      .SetObject(Object(make_shared<wstring>(wstr), kTypeIdWideString));
  }

  Message WideStringCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    wstring lhs = p[kStrObject].Cast<wstring>();
    bool result = false;
    if (rhs.GetTypeId() == kTypeIdWideString) {
      wstring rhs_wstr = rhs.Cast<wstring>();

      result = (lhs == rhs_wstr);
    }

    return Message().SetObject(result);
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

  Message FunctionCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    auto &lhs = p[kStrObject].Cast<Interface>();

    string type_id = rhs.GetTypeId();
    bool result = false;

    if (type_id == kTypeIdFunction) {
      auto &rhs_interface = rhs.Cast<Interface>();

      result = (lhs == rhs_interface);
    }
    
    return Message().SetObject(result);
  }

  void InitBaseTypes() {
    using management::type::NewTypeSetup;
    using management::CreateNewInterface;

    NewTypeSetup(kTypeIdFunction, SimpleSharedPtrCopy<Interface>)
      .InitMethods(
        {
          Interface(FunctionGetId, "", "id"),
          Interface(FunctionGetParameters, "", "params"),
          Interface(FunctionCompare, kStrRightHandSide, kStrCompare)
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
          Interface(StringCompare, kStrRightHandSide, kStrCompare),
          Interface(StringToArray, "","to_array")
        }
    );

    NewTypeSetup(kTypeIdInStream, FakeCopy<ifstream>)
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

    NewTypeSetup(kTypeIdOutStream, FakeCopy<ofstream>)
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

    NewTypeSetup(kTypeIdRegex, FakeCopy<regex>)
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