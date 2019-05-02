#include "string_util.h"

namespace kagami {
  inline bool IsStringFamily(Object &obj) {
    return compare(obj.GetTypeId(), kTypeIdString, kTypeIdWideString);
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

  Message CharFromInt(ObjectMap &p) {
    EXPECT_TYPE(p, "value", kTypeIdInt);
    auto value = static_cast<char>(p.Cast<int64_t>("value"));
    return Message().SetObject(string().append(1, value));
  }

  Message IntFromChar(ObjectMap &p) {
    EXPECT_TYPE(p, "value", kTypeIdString);
    auto &value = p.Cast<string>("value");

    if (value.size() != 1) {
      return Message(kCodeIllegalParam, "Invalid char", kStateError);
    }

    return Message().SetObject(static_cast<int64_t>(value[0]));
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

      base.PackContent(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag();
    }
    else if (obj.GetTypeId() == kTypeIdString) {
      string copy = obj.Cast<string>();
      base.PackContent(make_shared<string>(copy), kTypeIdString)
        .SetConstructorFlag();
    }
    else {
      string output = obj.Cast<string>();

      base.PackContent(make_shared<string>(output), kTypeIdString)
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

    for (auto &unit : str) {
      base->emplace_back(string().append(1, unit));
    }

    return Message().SetObject(Object(base, kTypeIdArray));
  }

  //InStream-new
  Message NewInStream(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    string path = p.Cast<string>("path");

    shared_ptr<InStream> ifs = make_shared<InStream>(path);

    return Message().SetObject(Object(ifs, kTypeIdInStream));
  }

  Message InStreamGet(ObjectMap &p) {
    InStream &ifs = p.Cast<InStream>(kStrMe);

    if (!ifs.Good()) {
      return Message(kCodeBadStream, "Invalid instream.", kStateError);
    }

    string result = ifs.GetLine();

    return Message().SetObject(result);
  }

  Message InStreamEOF(ObjectMap &p) {
    InStream &ifs = p.Cast<InStream>(kStrMe);
    return Message().SetObject(ifs.eof());
  }

  //OutStream
  Message NewOutStream(ObjectMap &p) {
    EXPECT_TYPE(p, "path", kTypeIdString);
    EXPECT_TYPE(p, "mode", kTypeIdString);

    string path = p.Cast<string>("path");
    string mode = p.Cast<string>("mode");

    shared_ptr<OutStream> ofs;
    bool append = (mode == "append");
    bool truncate = (mode == "truncate");

    if (!append && truncate) {
      ofs = make_shared<OutStream>(path, "w");
    }
    else if (append && !truncate) {
      ofs = make_shared<OutStream>(path, "a+");
    }

    return Message().SetObject(Object(ofs, kTypeIdOutStream));
  }

  Message OutStreamWrite(ObjectMap &p) {
    OutStream &ofs = p.Cast<OutStream>(kStrMe);
    auto &obj = p["str"];
    bool result = true;

    if (obj.GetTypeId() == kTypeIdString) {
      string str = obj.Cast<string>();
      result = ofs.WriteLine(str);
    }
    else {
      result = false;
    }
    
    return Message().SetObject(result);
  }

  //regex
  Message NewRegex(ObjectMap &p) {
    EXPECT_TYPE(p, "pattern", kTypeIdString);

    string pattern_string = p.Cast<string>("pattern");
    shared_ptr<regex> reg = make_shared<regex>(pattern_string);

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

  Message WideStringPrint(ObjectMap &p) {
    wstring &str = p.Cast<wstring>(kStrMe);
    OutStreamW(stdout).WriteLine(str);
    CHECK_PRINT_OPT();
    return Message();
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
      dest_base->emplace_back(Object(*it, kTypeIdString));
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
      .InitComparator(PlainComparator<Interface>)
      .InitMethods(
        {
          Interface(FunctionGetId, "", "id"),
          Interface(FunctionGetParameters, "", "params"),
          Interface(FunctionCompare, kStrRightHandSide, kStrCompare)
        }
    );

    NewTypeSetup(kTypeIdString, SimpleSharedPtrCopy<string>, PlainHasher<string>())
      .InitComparator(PlainComparator<string>)
      .InitConstructor(
        Interface(NewString, "raw_string", "string")
      )
      .InitMethods(
        {
          Interface(StringFamilyGetElement<string>, "index", "__at"),
          Interface(StringFamilySubStr<string>, "start|size", "substr"),
          Interface(GetStringFamilySize<string>, "", "size"),
          Interface(StringFamilyConverting<wstring, string>, "", "to_wide"),
          Interface(StringCompare, kStrRightHandSide, kStrCompare),
          Interface(StringToArray, "","to_array")
        }
    );

    NewTypeSetup(kTypeIdInStream, FakeCopy<InStream>, PointerHasher())
      .InitConstructor(
        Interface(NewInStream, "path", "instream")
      )
      .InitMethods(
        {
          Interface(InStreamGet, "", "get"),
          Interface(InStreamEOF, "", "eof"),
          Interface(StreamFamilyState<InStream>, "", "good"),
        }
    );

    NewTypeSetup(kTypeIdOutStream, FakeCopy<OutStream>, PointerHasher())
      .InitConstructor(
        Interface(NewOutStream, "path|mode", "outstream")
      )
      .InitMethods(
        {
          Interface(OutStreamWrite, "str", "write"),
          Interface(StreamFamilyState<OutStream>, "", "good"),
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
      .InitComparator(PlainComparator<wstring>)
      .InitConstructor(
        Interface(NewWideString, "raw_string", "wstring")
      )
      .InitMethods(
        {
          Interface(GetStringFamilySize<wstring>,  "", "size"),
          Interface(StringFamilyGetElement<wstring>, "index", "__at"),
          Interface(WideStringPrint, "", "print"),
          Interface(StringFamilySubStr<wstring>, "start|size", "substr"),
          Interface(StringFamilyConverting<string, wstring>, "", "to_byte"),
          Interface(WideStringCompare, kStrRightHandSide, kStrCompare)
        }
    );

    CreateNewInterface(Interface(DecimalConvert<2>, "str", "bin"));
    CreateNewInterface(Interface(DecimalConvert<8>, "str", "octa"));
    CreateNewInterface(Interface(DecimalConvert<16>, "str", "hex"));
    CreateNewInterface(Interface(CreateStringFromArray, "src", "ar2string"));
    CreateNewInterface(Interface(CharFromInt, "value", "int2str"));
    CreateNewInterface(Interface(IntFromChar, "value", "str2int"));

    EXPORT_CONSTANT(kTypeIdFunction);
    EXPORT_CONSTANT(kTypeIdString);
    EXPORT_CONSTANT(kTypeIdInStream);
    EXPORT_CONSTANT(kTypeIdOutStream);
    EXPORT_CONSTANT(kTypeIdRegex);
    EXPORT_CONSTANT(kTypeIdWideString);
  }
}