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
    string typeId = p[kStrObject].GetTypeId();
    string methods = p[kStrObject].GetMethods();
    int start = stoi(p.Get<string>("start"));
    int size = stoi(p.Get<string>("size"));
    Message msg;

    if (start < 0 || size > int(str.size()) - start) {
      msg = Message(kStrFatalError, kCodeIllegalParm, "Illegal index or size.");
    }
    else {
      Object ret;
      StringType output = str.substr(start, size);

      ret.Set(make_shared<StringType>(output), typeId)
        .SetMethods(methods)
        .SetRo(false);
      msg.SetObject(ret);
    }

    return msg;
  }

  template <class StringType>
  Message StringFamilyGetElement(ObjectMap &p) {
    StringType &str = p.Get<StringType>(kStrObject);
    string typeId = p[kStrObject].GetTypeId();
    string methods = p[kStrObject].GetMethods();
    int size = int(str.size());
    int idx = stoi(p.Get<string>("index"));
    Message msg;

    if (idx > size && idx >= 0) {
      shared_ptr<StringType> output = make_shared<StringType>();
      Object ret;

      output->append(1, str[idx]);
      ret.Set(output, typeId, methods, false);
      msg.SetObject(ret);
    }
    else {
      msg = Message(kStrFatalError, kCodeIllegalParm, "Index out of range.");
    }

    return msg;
  }

  template <class StringType, class StreamType>
  class StreamBase {
    StreamType *stream;
  public:
    StreamType &operator<<(StringType &str) { return *stream; }
    StreamBase(){}
  };

  template <>
  class StreamBase<string, std::ostream> {
    std::ostream *stream;
  public:
    std::ostream &operator<<(string &str) {
      *stream << str;
      return *stream;
    }

    StreamBase() { stream = &std::cout; }
  };

  template<>
  class StreamBase<wstring, std::wostream> {
    std::wostream *stream;
  public:
    std::wostream &operator<<(wstring &str) {
      *stream << str;
      return *stream;
    }
    StreamBase() { stream = &std::wcout; }
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
    Kit::MakeBoolean(stream.good(), temp);
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
    bool isWide = std::is_same<DestType, string>::value;
    string typeId, methods;
    Message msg;
    Object ret;

    if (isWide) {
      typeId = kTypeIdWideString;
      methods = kWideStringMethods;
    }
    else {
      typeId = kTypeIdString;
      methods = kStringMethods;
    }

    ret.Set(dest, typeId, methods, false);
    msg.SetObject(ret);
    return msg;
  }
}