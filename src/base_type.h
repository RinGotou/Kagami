#pragma once
#include "machine.h"

namespace kagami {
  using ArrayBase = vector<Object>;
  template <class StringType>
  Message GetStringFamilySize(ObjectMap &p) {
    StringType &str = p.Get<StringType>(kStrObject);
    return Message(to_string(str.size()));
  }
  
  template <class StringType>
  Message StringFamilySubStr(ObjectMap &p) {
    StringType &str = p.Get<StringType>(kStrObject);
    string type_id = p[kStrObject].GetTypeId();
    string methods = p[kStrObject].GetMethods();
    int start = stoi(p.Get<string>("start"));
    int size = stoi(p.Get<string>("size"));

    CONDITION_ASSERT((start >= 0 && size <= int(str.size()) - start),
      "Illegal index or size.");

    StringType output = str.substr(start, size);

    return Message()
      .SetObject(Object(make_shared<StringType>(output), type_id, methods, false));
  }

  template <class StringType>
  Message StringFamilyGetElement(ObjectMap &p) {
    StringType &str = p.Get<StringType>(kStrObject);
    string type_id = p[kStrObject].GetTypeId();
    string methods = p[kStrObject].GetMethods();
    int size = int(str.size());
    int idx = stoi(p.Get<string>("index"));

    CONDITION_ASSERT((idx > size && idx >= 0), "Index out of range.");

    shared_ptr<StringType> output = make_shared<StringType>();

    output->append(1, str[idx]);
    return Message().SetObject(Object(output, type_id, methods, false));
  }

  template <class StringType, class StreamType>
  class StreamBase {
    StreamType *stream_;
  public:
    StreamType &operator<<(StringType &str) { return *stream_; }
    StreamBase(){}
  };

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
    StringType &str = p.Get<StringType>(kStrObject);
    StreamBase<StringType, StreamType> stream;
    stream << str << std::endl;
    return Message();
  }

  template <class StreamType>
  Message StreamFamilyClose(ObjectMap &p) {
    StreamType &stream = p.Get<StreamType>(kStrObject);
    stream.close();
    return Message();
  }

  template <class StreamType>
  Message StreamFamilyState(ObjectMap &p) {
    StreamType &stream = p.Get<StreamType>(kStrObject);
    string temp;
    util::MakeBoolean(stream.good(), temp);
    return Message(temp);
  }

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

  template<class DestType,class SrcType>
  Message StringFamilyConverting(ObjectMap &p) {
    StringConvertor<DestType, SrcType> convertor;
    SrcType &str = p.Get<SrcType>(kStrObject);
    shared_ptr<DestType> dest = make_shared<DestType>(convertor(str));
    string type_id, methods;

    if (std::is_same<DestType, wstring>::value) {
      type_id = kTypeIdWideString;
      methods = kWideStringMethods;
    }
    else {
      type_id = kTypeIdString;
      methods = kStringMethods;
    }

    return Message().SetObject(Object(dest, type_id, methods, false));
  }
}