#pragma once
#include "machine.h"

namespace kagami {
  template <typename StringType>
  Message GetStringFamilySize(ObjectMap &p) {
    StringType &str = p.Cast<StringType>(kStrMe);
    int64_t size = static_cast<int64_t>(str.size());
    return Message().SetObject(size);
  }
  
  template <typename StringType>
  Message StringFamilySubStr(ObjectMap &p) {
    StringType &str = p.Cast<StringType>(kStrMe);

    string type_id = p[kStrMe].GetTypeId();

    int64_t start = p.Cast<int64_t>("start");
    int64_t size = p.Cast<int64_t>("size");

    if (start < 0 || size > static_cast<int64_t>(str.size() - start)) {
      return Message("Invalid index/size", kStateError);
    }

    StringType output = str.substr(start, size);

    return Message().SetObject(Object(make_shared<StringType>(output), type_id));
  }

  template <typename StringType>
  Message StringFamilyGetElement(ObjectMap &p) {
    StringType &str = p.Cast<StringType>(kStrMe);
    string type_id = p[kStrMe].GetTypeId();

    size_t size = str.size();
    size_t idx = p.Cast<int64_t>("index");

    if (idx >= size || idx < 0) return Message("Index is out of range", kStateError);

    shared_ptr<StringType> output = make_shared<StringType>();

    output->append(1, str[idx]);
    return Message().SetObject(Object(output, type_id));
  }

  template<typename DestType,class SrcType>
  Message StringFamilyConverting(ObjectMap &p) {
    SrcType &str = p.Cast<SrcType>(kStrMe); 
    string type_id;
    Message msg;
    shared_ptr<DestType> dest;

    if constexpr (is_same<DestType, wstring>::value) {
      dest = make_shared<wstring>(s2ws(str));
      type_id = kTypeIdWideString;
    }
    else if constexpr (is_same<DestType, string>::value) {
      dest = make_shared<string>(ws2s(str));
      type_id = kTypeIdString;
    }

    msg.SetObject(Object(dest, type_id));
    return msg;
  }

  template <int base>
  Message DecimalConvert(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("str", kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

    string str = ParseRawString(p["str"].Cast<string>());

    int64_t dest = stol(str, nullptr, base);
    return Message().SetObject(Object(
      make_shared<int64_t>(dest), kTypeIdInt
    ));
  }
}