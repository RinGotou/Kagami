#pragma once
#include "machine.h"

namespace kagami {
  template <class StringType>
  Message GetStringFamilySize(ObjectMap &p) {
    StringType &str = p.Cast<StringType>(kStrMe);
    int64_t size = static_cast<int64_t>(str.size());
    return Message().SetObject(size);
  }
  
  template <class StringType>
  Message StringFamilySubStr(ObjectMap &p) {
    StringType &str = p.Cast<StringType>(kStrMe);

    string type_id = p[kStrMe].GetTypeId();

    int64_t start = p.Cast<int64_t>("start");
    int64_t size = p.Cast<int64_t>("size");

    EXPECT((start >= 0 && size <= static_cast<int64_t>(str.size() - start)),
      "Illegal index or size.");

    StringType output = str.substr(start, size);

    return Message().SetObject(Object(make_shared<StringType>(output), type_id));
  }

  template <class StringType>
  Message StringFamilyGetElement(ObjectMap &p) {
    StringType &str = p.Cast<StringType>(kStrMe);
    string type_id = p[kStrMe].GetTypeId();

    size_t size = str.size();
    size_t idx = p.Cast<int64_t>("index");

    EXPECT((idx < size && idx >= 0), "Index out of range.");

    shared_ptr<StringType> output = make_shared<StringType>();

    output->append(1, str[idx]);
    return Message().SetObject(Object(output, type_id));
  }

  /* Automatic stream chooser for different string type */
  /* Base class for stream chooser (DO NOT USE IT) */
  template <class StringType, class StreamType>
  class StreamBase {
    StreamType *stream_;
  public:
    StreamType &operator<<(StringType &str) { return *stream_; }
    StreamBase(){}
  };

  /* Stream chooser (for string) */
  template <>
  class StreamBase<string, std::ostream> {
    std::ostream *stream_;
  public:
    std::ostream &operator<<(string &str) {
      *stream_ << str;
      return *stream_;
    }

    StreamBase() { stream_ = &std::cout; }
  };

  /* String chooser (for wstring) */
  template<>
  class StreamBase<wstring, std::wostream> {
    std::wostream *stream_;
  public:
    std::wostream &operator<<(wstring &str) {
      *stream_ << str;
      return *stream_;
    }
    StreamBase() { stream_ = &std::wcout; }
  };

  template <class StringType, class StreamType>
  Message StringFamilyPrint(ObjectMap &p) {
    StringType &str = p.Cast<StringType>(kStrMe);
    StreamBase<StringType, StreamType> stream;
    stream << str;
    CHECK_PRINT_OPT();
    return Message();
  }

  template <class StreamType>
  Message StreamFamilyState(ObjectMap &p) {
    StreamType &stream = p.Cast<StreamType>(kStrMe);
    return Message().SetObject(stream.Good());
  }

  /* Convert string to destination type */
  template <class DestType, class SrcType>
  class StringConvertor {
  public:
    DestType operator()(const SrcType &src) { return DestType(); }
  };

  template<>
  class StringConvertor<wstring, string> {
  public:
    wstring operator()(const string &src) { return s2ws(src); }
  };

  template<>
  class StringConvertor<string, wstring> {
  public:
    string operator()(const wstring &src) { return ws2s(src); }
  };

  /* Policy for choosing right type identifier string */
  template<bool is_wstring>
  class ConvertingInfoPolicy {};

  template<>
  class ConvertingInfoPolicy<true> {
  public:
    string TypeId() { return kTypeIdWideString; }
  };

  template<>
  class ConvertingInfoPolicy<false> {
  public:
    string TypeId() { return kTypeIdString; }
  };

  template<class DestType,class SrcType>
  Message StringFamilyConverting(ObjectMap &p) {
    // Init convertor
    StringConvertor<DestType, SrcType> convertor; 
    // Get original string
    SrcType &str = p.Cast<SrcType>(kStrMe); 
    // Make result
    shared_ptr<DestType> dest = make_shared<DestType>(convertor(str)); 
    // Init object type identifier
    ConvertingInfoPolicy<std::is_same<DestType, wstring>::value> info_policy; 
    // Return result object
    return Message().SetObject(Object(dest, info_policy.TypeId()));
  }

  template <int base>
  Message DecimalConvert(ObjectMap &p) {
    EXPECT_TYPE(p, "str", kTypeIdString);
    string str = ParseRawString(p["str"].Cast<string>());

    int64_t dest = stol(str, nullptr, base);
    return Message().SetObject(Object(
      make_shared<int64_t>(dest), kTypeIdInt
    ));
  }
}