#include "string_obj.h"

namespace kagami {
  inline bool IsStringFamily(Object &obj) {
    return compare(obj.GetTypeId(), kTypeIdString, kTypeIdWideString);
  }

  Message CreateStringFromArray(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("src", kTypeIdArray) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

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
    auto tc = TypeChecking({ Expect("value", kTypeIdInt) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto value = static_cast<char>(p.Cast<int64_t>("value"));
    return Message().SetObject(string().append(1, value));
  }

  Message IntFromChar(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("value", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &value = p.Cast<string>("value");

    if (value.size() != 1) {
      return Message("Invalid char", kStateError);
    }

    return Message().SetObject(static_cast<int64_t>(value[0]));
  }

  //String
  Message NewString(ObjectMap &p) {
    Object &obj = p["raw_string"];
    Object base;

    if (!IsStringFamily(obj)) {
      return Message("String constructor cannot accept non-string obejct.", kStateError);
    }

    if (obj.GetTypeId() == kTypeIdWideString) {
      wstring wstr = obj.Cast<wstring>();
      string output = ws2s(wstr);

      base.PackContent(make_shared<string>(output), kTypeIdString);
    }
    else if (obj.GetTypeId() == kTypeIdString) {
      string copy = obj.Cast<string>();
      base.PackContent(make_shared<string>(copy), kTypeIdString);
    }
    else {
      string output = obj.Cast<string>();

      base.PackContent(make_shared<string>(output), kTypeIdString);
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

  //wstring
  Message NewWideString(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("raw_string", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

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
    OutStreamW(VM_STDOUT).Write(str);
    CHECK_PRINT_OPT(p);
    return Message();
  }

  void InitBaseTypes() {
    using management::CreateImpl;
    using namespace management::type;

    ObjectTraitsSetup(kTypeIdString, PlainDeliveryImpl<string>, PlainHasher<string>)
      .InitComparator(PlainComparator<string>)
      .InitConstructor(
        FunctionImpl(NewString, "raw_string", "string")
      )
      .InitMethods(
        {
          FunctionImpl(StringFamilyGetElement<string>, "index", kStrAt),
          FunctionImpl(StringFamilySubStr<string>, "start|size", "substr"),
          FunctionImpl(GetStringFamilySize<string>, "", "size"),
          FunctionImpl(StringFamilyConverting<wstring, string>, "", "to_wide"),
          FunctionImpl(StringCompare, kStrRightHandSide, kStrCompare),
          FunctionImpl(StringToArray, "","to_array")
        }
    );

    ObjectTraitsSetup(kTypeIdWideString, PlainDeliveryImpl<wstring>, PlainHasher<wstring>)
      .InitComparator(PlainComparator<wstring>)
      .InitConstructor(
        FunctionImpl(NewWideString, "raw_string", "wstring")
      )
      .InitMethods(
        {
          FunctionImpl(GetStringFamilySize<wstring>,  "", "size"),
          FunctionImpl(StringFamilyGetElement<wstring>, "index", kStrAt),
          FunctionImpl(WideStringPrint, "", "print"),
          FunctionImpl(StringFamilySubStr<wstring>, "start|size", "substr"),
          FunctionImpl(StringFamilyConverting<string, wstring>, "", "to_byte"),
          FunctionImpl(WideStringCompare, kStrRightHandSide, kStrCompare)
        }
    );

    CreateImpl(FunctionImpl(DecimalConvert<2>, "str", "bin"));
    CreateImpl(FunctionImpl(DecimalConvert<8>, "str", "octa"));
    CreateImpl(FunctionImpl(DecimalConvert<16>, "str", "hex"));
    CreateImpl(FunctionImpl(CreateStringFromArray, "src", "ar2string"));
    CreateImpl(FunctionImpl(CharFromInt, "value", "int2str"));
    CreateImpl(FunctionImpl(IntFromChar, "value", "str2int"));


    EXPORT_CONSTANT(kTypeIdString);
    EXPORT_CONSTANT(kTypeIdWideString);
  }
}