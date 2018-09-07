#include "base_type.h"
#include <iostream>

namespace kagami {
  //components

  //Common
  shared_ptr<void> NoCopy(shared_ptr<void> target) {
    return target;
  }

  //Null
  shared_ptr<void> NullCopy(shared_ptr<void> target) {
    return make_shared<int>(0);
  }

  //Array
  shared_ptr<void> ArrayCopy(shared_ptr<void> target) {
    const auto ptr = static_pointer_cast<vector<Object>>(target);
    vector<Object> base = *ptr;
    return make_shared<vector<Object>>(std::move(base));
  }

  Message ArrayConstructor(ObjectMap &p) {
    Message result;
    auto size = stoi(GetObjectStuff<string>(p["size"]));
    Object obj = Object();
    auto it = p.find("init_value");
    if (it != p.end()) {
      obj.Copy(it->second);
      obj.SetRo(false);
    }
    ArrayBase base;

    if (size <= 0) {
      result.combo(kStrFatalError, kCodeIllegalParm, "Illegal array size.");
      return result;
    }

    const auto typeId = obj.GetTypeId();
    const auto methods = obj.GetMethods();
    const auto tokenTypeEnum = obj.GetTokenType();
    shared_ptr<void> initPtr;
    base.reserve(size);

    for (auto count = 0; count < size; count++) {
      initPtr = type::GetObjectCopy(obj);
      base.emplace_back((Object()
        .Set(initPtr, typeId)
        .SetMethods(methods)
        .SetTokenType(tokenTypeEnum)
        .SetRo(false)));
    }

    result.SetObject(Object()
      .SetConstructorFlag()
      .Set(make_shared<ArrayBase>(base), kTypeIdArrayBase)
      .SetMethods(kArrayBaseMethods)
      .SetRo(false)
      , "__result");
    return result;
  }

  Message ArrayGetElement(ObjectMap &p) {
    Object &obj = p[kStrObject];
    int idx = stoi(GetObjectStuff<string>(p["index"]));
    auto &base = GetObjectStuff<ArrayBase>(obj);
    int size = int(base.size());
    Message msg;
    if (idx < size) {
      Object temp;
      temp.Ref(base[idx]);
      msg.SetObject(temp, "__element");
    }
    else {
      msg.combo(kStrFatalError, kCodeOverflow, "Subscript is out of range");
    }
    return msg;
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrObject];
    return Message(kStrRedirect, kCodeSuccess, to_string(static_pointer_cast<ArrayBase>(obj.Get())->size()));
  }

  Message ArrayPrint(ObjectMap &p) {
    Message result;
    Object object = p[kStrObject];
    ObjectMap map;
    if (object.GetTypeId() == kTypeIdArrayBase) {
      auto &base = *static_pointer_cast<vector<Object>>(object.Get());
      auto ent = entry::Order("print", kTypeIdNull, -1);
      for (auto &unit : base) {
        map.insert(pair<string, Object>(kStrObject, unit));
        result = ent.Start(map);
        map.clear();
      }
    }
    return result;
  }
  
  //RawString
  Message RawStringGetElement(ObjectMap &p) {
    Message result;
    Object temp;
    size_t size;
    auto obj = p.at(kStrObject), objIdx = p["index"];
    const auto typeId = obj.GetTypeId();
    int idx = stoi(*static_pointer_cast<string>(objIdx.Get()));

    const auto makeStrToken = [](char target)->string {
      return string().append("'").append(1, target).append("'");
    };

    auto data = *static_pointer_cast<string>(obj.Get());
    if (Kit::IsString(data)) {
      data = Kit::GetRawString(data);
    }
    size = data.size();
    if (idx <= int(size - 1)) {
      result.combo(kStrRedirect, kCodeSuccess, makeStrToken(data.at(idx)));
    }
    else {
      result.combo(kStrFatalError, kCodeOverflow, "Subscript is out of range");
    }
    
    return result;
  }

  Message RawStringGetSize(ObjectMap &p) {
    auto &object = p.at(kStrObject);
    auto str = *static_pointer_cast<string>(object.Get());
    if (Kit::IsString(str)) str = Kit::GetRawString(str);
    return Message(kStrRedirect, kCodeSuccess, to_string(str.size()));
  }

  Message RawStringPrint(ObjectMap &p) {
    Message result;
    auto &object = p[kStrObject];
    bool doNotWrap = (p.find("not_wrap") != p.end());
    string msg;
    auto needConvert = false;

    auto data = *static_pointer_cast<string>(object.Get());
    if (Kit::IsString(data)) data = Kit::GetRawString(data);
    std::cout << data;
    if (!doNotWrap) std::cout << std::endl;
    
    return result;
  }

  //String
  Message StringConstructor(ObjectMap &p) {
    Object &obj = p["raw_string"];
    Object base;
    if (obj.GetTypeId() != kTypeIdRawString 
      && obj.GetTypeId() != kTypeIdString
      && obj.GetTypeId() != kTypeIdWideString) {
      return Message(kStrFatalError, kCodeIllegalParm, "String constructor can't accept this object.");
    }
    if (obj.GetTypeId() == kTypeIdWideString) {
      wstring wstr = GetObjectStuff<wstring>(obj);
      string output = ws2s(wstr);
      base.Set(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag()
        .SetMethods(kStringMethods)
        .SetRo(false);
    }
    else if (obj.GetTypeId() == kTypeIdString) {
      base.Set(obj.Get(), kTypeIdString)
        .SetConstructorFlag()
        .SetMethods(kStringMethods)
        .SetRo(false);
    }
    else {
      string origin = GetObjectStuff<string>(obj);
      string output;
      if (Kit::IsString(origin)) {
        output = Kit::GetRawString(origin);
      }
      else {
        output = origin;
      }
      base.Set(make_shared<string>(output), kTypeIdString)
        .SetConstructorFlag()
        .SetMethods(kStringMethods)
        .SetRo(false);
    }

    Message msg;
    msg.SetObject(base, "__result");
    return msg;
  }

  Message StringGetSize(ObjectMap &p) {
    string &str = GetObjectStuff<string>(p[kStrObject]);
    return Message(kStrRedirect, kCodeSuccess, to_string(str.size()));
  }

  Message StringGetElement(ObjectMap &p) {
    string &str = GetObjectStuff<string>(p[kStrObject]);
    int idx = stoi(GetObjectStuff<string>(p["index"]));
    int size = int(str.size());
    Message msg;
    if (idx < size && idx >= 0) {
      msg.combo(kStrRedirect, kCodeSuccess, to_string(str[idx]));
    }
    else {
      msg.combo(kStrFatalError, kCodeIllegalParm, "Illegal index.");
    }
    return msg;
  }

  Message StringPrint(ObjectMap &p) {
    string &str = GetObjectStuff<string>(p[kStrObject]);
    std::cout << str << std::endl;
    return Message();
  }

  Message StringSubStr(ObjectMap &p) {
    string &str = GetObjectStuff<string>(p[kStrObject]);
    int start = stoi(GetObjectStuff<string>(p["start"])),
      size = stoi(GetObjectStuff<string>(p["size"]));
    Message msg;
    if (start < 0 || size > str.size() - start) {
      msg.combo(kStrFatalError, kCodeIllegalParm, "Illegal index or size.");
    }
    else {
      string output = str.substr(start, size);
      Object obj;
      obj.Set(make_shared<string>(output), kTypeIdString)
        .SetMethods(kStringMethods)
        .SetRo(true);
      msg.SetObject(obj, "__result");
    }
    return msg;
  }

  Message StringToWide(ObjectMap &p) {
    Object &obj = p[kStrObject];
    string origin = GetObjectStuff<string>(obj);
    shared_ptr<wstring> wstr = make_shared<wstring>(s2ws(origin));
    Object base;
    base.Set(wstr, kTypeIdWideString)
      .SetMethods(kWideStringMethods)
      .SetRo(false);
    Message msg;
    msg.SetObject(base, "__result");
    return msg;
  }

  //InStream
  Message InStreamConsturctor(ObjectMap &p) {
    Object &objPath = p["path"];
    //TODO:support for string type
    string path = Kit::GetRawString(GetObjectStuff<string>(objPath));
    shared_ptr<ifstream> ifs = make_shared<ifstream>(ifstream(path.c_str(), std::ios::in));
    Message msg;
    Object obj;
    obj.Set(ifs, kTypeIdInStream)
      .SetMethods(kInStreamMethods)
      .SetRo(false);
    msg.SetObject(obj, "__result");
    return msg;
  }

  Message InStreamGet(ObjectMap &p) {
    Object &obj = p[kStrObject];
    ifstream &ifs = GetObjectStuff<ifstream>(obj);
    Message msg;
    if (ifs.eof()) {
      msg.combo(kStrRedirect, kCodeSuccess, "");
    }
    if (ifs.good()) {
      string str;
      std::getline(ifs, str);
      Object obj;
      obj.Set(make_shared<string>(str), kTypeIdString).SetMethods(kStringMethods).SetRo(false);
      msg.SetObject(obj, "__result");
    }
    else {
      msg.combo(kStrFatalError, kCodeBadStream, "InStream is not working.");
    }
    return msg;
  }

  Message InStreamGood(ObjectMap &p) {
    Object &obj = p[kStrObject];
    ifstream &ifs = GetObjectStuff<ifstream>(obj);
    string state;
    ifs.good() ? state = kStrTrue : state = kStrFalse;
    Message msg(kStrRedirect, kCodeSuccess, state);
    return msg;
  }

  Message InStreamEOF(ObjectMap &p) {
    Object &obj = p[kStrObject];
    ifstream &ifs = GetObjectStuff<ifstream>(obj);
    string state;
    ifs.eof() ? state = kStrTrue : state = kStrFalse;
    Message msg(kStrRedirect, kCodeSuccess, state);
    return msg;
  }

  Message InStreamClose(ObjectMap &p) {
    Object &obj = p[kStrObject];
    ifstream &ifs = GetObjectStuff<ifstream>(obj);
    ifs.close();
    return Message();
  }

  //OutStream
  Message OutStreamConstructor(ObjectMap &p) {
    Object &objPath = p["path"];
    Object &objMode = p["mode"];
    string path = Kit::GetRawString(GetObjectStuff<string>(objPath));
    string mode = Kit::GetRawString(GetObjectStuff<string>(objMode));
    Message msg;
    shared_ptr<ofstream> ofs;
    bool append = false;
    bool truncate = false;
    if (mode == "append") append = true;
    else if (mode == "truncate") truncate = true;
    if (!append && truncate) {
      ofs = make_shared<ofstream>(ofstream(path.c_str(), std::ios::out | std::ios::trunc));
    }
    else {
      ofs = make_shared<ofstream>(ofstream(path.c_str(), std::ios::out | std::ios::app));
    }
    Object obj;
    obj.Set(ofs, kTypeIdOutStream)
      .SetMethods(kOutStreamMethods)
      .SetRo(false);
    msg.SetObject(obj, "__result");
    return msg;
  }

  Message OutStreamWrite(ObjectMap &p) {
    Object &obj = p[kStrObject];
    ofstream &ofs = GetObjectStuff<ofstream>(obj);
    Object &objStr = p["str"];
    Message msg;
    if (!ofs.good()) {
      return Message(kStrRedirect, kCodeSuccess, kStrFalse);
    }
    if (objStr.GetTypeId() == kTypeIdRawString) {
      string output;
      string &origin = GetObjectStuff<string>(objStr);
      if (Kit::IsString(origin)) {
        output = Kit::GetRawString(origin);
      }
      ofs << output;
    }
    else if (objStr.GetTypeId() == kTypeIdString) {
      string &origin = GetObjectStuff<string>(objStr);
      ofs << origin;
    }
    else {
      msg.combo(kStrRedirect, kCodeSuccess, kStrFalse);
    }
    return msg;
  }

  Message OutStreamGood(ObjectMap &p) {
    Object &obj = p[kStrObject];
    ofstream &ofs = GetObjectStuff<ofstream>(obj);
    string state;
    ofs.good() ? state = kStrTrue : state = kStrFalse;
    Message msg(kStrRedirect, kCodeSuccess, state);
    return msg;
  }

  Message OutStreamClose(ObjectMap &p) {
    Object &obj = p[kStrObject];
    ofstream &ofs = GetObjectStuff<ofstream>(obj);
    ofs.close();
    return Message();
  }

  //regex
  Message RegexConstructor(ObjectMap &p) {
    Object &obj = p["regex"];
    string regStr = GetObjectStuff<string>(obj);
    Kit::IsString(regStr) ? regStr = Kit::GetRawString(regStr): regStr = regStr;
    shared_ptr<regex> reg = make_shared<regex>(regex(regStr));
    Object ret;
    ret.Set(reg, kTypeIdRegex)
      .SetMethods(kRegexMethods)
      .SetRo(false);
    Message msg;
    msg.SetObject(ret, "__result");
    return msg;
  }

  Message RegexMatch(ObjectMap &p) {
    Object &obj = p["str"];
    string str = GetObjectStuff<string>(obj);
    Kit::IsString(str) ? str = Kit::GetRawString(str) : str = str;
    auto &pat = GetObjectStuff<regex>(p[kStrObject]);
    string state;
    regex_match(str, pat) ? state = kStrTrue : state = kStrFalse;
    return Message(kStrRedirect, kCodeSuccess, state);
  }

  //wstring
  Message WideStringContructor(ObjectMap &p) {
    Object obj = p["raw_string"];
    if (obj.GetTypeId() != kTypeIdRawString && obj.GetTypeId() != kTypeIdString) {
      return Message(kStrFatalError, kCodeIllegalParm, "String constructor can't accept this object.");
    }
    Object base;
    string origin = GetObjectStuff<string>(obj);
    string output;
    if (Kit::IsString(origin)) output = origin.substr(1, origin.size() - 2);
    else output = origin;
    wstring wstr = s2ws(output);
    base.Set(make_shared<wstring>(wstr), kTypeIdWideString)
      .SetMethods(kWideStringMethods)
      .SetRo(false);
    Message msg;
    msg.SetObject(base, "__result");
    return msg;
  }

  Message WideStringGetSize(ObjectMap &p) {
    Object &obj = p[kStrObject];
    wstring &wstr = GetObjectStuff<wstring>(obj);
    return Message(kStrRedirect, kCodeSuccess, to_string(wstr.size()));
  }

  Message WideStringGetElement(ObjectMap &p) {
    Object &obj = p[kStrObject], objIdx = p["index"];
    wstring &wstr = GetObjectStuff<wstring>(obj);
    int size = int(wstr.size());
    int idx = stoi(GetObjectStuff<string>(objIdx));
    Message msg;
    if (idx < size && idx >= 0) {
      wstring output;
      output.append(1, wstr[idx]);
      Object ret;
      ret.Set(make_shared<wstring>(output), kTypeIdWideString)
        .SetMethods(kWideStringMethods)
        .SetRo(false);
      msg.SetObject(ret, "__result");
    }
    else {
      msg.combo(kStrFatalError, kCodeIllegalParm, "Index out of range.");
    }
    return msg;
  }

  Message WideStringPrint(ObjectMap &p) {
    Object &obj = p[kStrObject];
    wstring &wstr = GetObjectStuff<wstring>(obj);
    std::wcout << wstr << std::endl;
    return Message();
  }

  Message WideStringSubStr(ObjectMap &p) {
    Object &obj = p[kStrObject], objStart = p["start"], objSize = p["size"];
    wstring &wstr = GetObjectStuff<wstring>(obj);
    int start = stoi(GetObjectStuff<string>(objStart)),
      size = stoi(GetObjectStuff<string>(objSize));
    Message msg;
    if (start < 0 || size > wstr.size() - start) {
      msg.combo(kStrFatalError, kCodeIllegalParm, "Illegal index or size.");
    }
    else {
      Object ret;
      wstring output = wstr.substr(start, size);
      ret.Set(make_shared<wstring>(output), kTypeIdWideString)
        .SetMethods(kWideStringMethods)
        .SetRo(false);
      msg.SetObject(ret, "__result");
    }
    return msg;
  }

  Message WideStringToByte(ObjectMap &p) {
    Object &obj = p[kStrObject];
    wstring &wstr = GetObjectStuff<wstring>(obj);
    shared_ptr<string> str = make_shared<string>(string(ws2s(wstr)));
    Object ret;
    ret.Set(str, kTypeIdString)
      .SetMethods(kStringMethods)
      .SetRo(false);
    Message msg;
    msg.SetObject(ret, "__result");
    return msg;
  }

  void InitPlanners() {
    using type::AddTemplate;
    using entry::AddEntry;
    AddTemplate(kTypeIdRawString, ObjectPlanner(SimpleSharedPtrCopy<string>, kRawStringMethods));
    AddEntry(Entry(RawStringPrint, kCodeNormalParm, "", "__print", kTypeIdRawString, kFlagMethod));
    AddEntry(Entry(RawStringGetElement, kCodeNormalParm, "subscript", "__at", kTypeIdRawString, kFlagMethod));
    AddEntry(Entry(RawStringGetSize, kCodeNormalParm, "", "size", kTypeIdRawString, kFlagMethod));

    AddTemplate(kTypeIdArrayBase, ObjectPlanner(ArrayCopy, kArrayBaseMethods));
    AddEntry(Entry(ArrayConstructor, kCodeAutoFill, "size|init_value", "array"));
    AddEntry(Entry(ArrayGetElement, kCodeNormalParm, "index", "__at", kTypeIdArrayBase, kFlagMethod));
    AddEntry(Entry(ArrayPrint, kCodeNormalParm, "", "__print", kTypeIdArrayBase, kFlagMethod));
    AddEntry(Entry(ArrayGetSize, kCodeNormalParm, "", "size", kTypeIdArrayBase, kFlagMethod));

    AddTemplate(kTypeIdString, ObjectPlanner(SimpleSharedPtrCopy<string>, kStringMethods));
    AddEntry(Entry(StringConstructor, kCodeNormalParm, "raw_string", "string"));
    AddEntry(Entry(StringGetElement, kCodeNormalParm, "index", "__at", kTypeIdString, kFlagMethod));
    AddEntry(Entry(StringPrint, kCodeNormalParm, "", "__print", kTypeIdString, kFlagMethod));
    AddEntry(Entry(StringSubStr, kCodeNormalParm, "start|size", "substr", kTypeIdString, kFlagMethod));
    AddEntry(Entry(StringGetSize, kCodeNormalParm, "", "size", kTypeIdString, kFlagMethod));
    AddEntry(Entry(StringToWide, kCodeNormalParm, "", "to_wide", kTypeIdString, kFlagMethod));

    AddTemplate(kTypeIdInStream, ObjectPlanner(NoCopy, kInStreamMethods));
    AddEntry(Entry(InStreamConsturctor, kCodeNormalParm, "path", "instream"));
    AddEntry(Entry(InStreamGet, kCodeNormalParm, "", "get", kTypeIdInStream, kFlagMethod));
    AddEntry(Entry(InStreamGood, kCodeNormalParm, "", "good", kTypeIdInStream, kFlagMethod));
    AddEntry(Entry(InStreamEOF, kCodeNormalParm, "", "eof", kTypeIdInStream, kFlagMethod));
    AddEntry(Entry(InStreamClose, kCodeNormalParm, "", "close", kTypeIdInStream, kFlagMethod));

    AddTemplate(kTypeIdOutStream, ObjectPlanner(NoCopy, kOutStreamMethods));
    AddEntry(Entry(OutStreamConstructor, kCodeNormalParm, "path|mode", "outstream"));
    AddEntry(Entry(OutStreamWrite, kCodeNormalParm, "str", "write", kTypeIdOutStream, kFlagMethod));
    AddEntry(Entry(OutStreamGood, kCodeNormalParm, "", "good", kTypeIdOutStream, kFlagMethod));
    AddEntry(Entry(OutStreamClose, kCodeNormalParm, "", "close", kTypeIdOutStream, kFlagMethod));

    AddTemplate(kTypeIdRegex, ObjectPlanner(NoCopy, kTypeIdRegex));
    AddEntry(Entry(RegexConstructor, kCodeNormalParm, "regex", "regex"));
    AddEntry(Entry(RegexMatch, kCodeNormalParm, "str", "match", kTypeIdRegex, kFlagMethod));

    AddTemplate(kTypeIdString, ObjectPlanner(SimpleSharedPtrCopy<wstring>, kWideStringMethods));
    AddEntry(Entry(WideStringContructor, kCodeNormalParm, "raw_string", "wstring"));
    AddEntry(Entry(WideStringGetSize, kCodeNormalParm, "", "size", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(WideStringGetElement, kCodeNormalParm, "index", "__at", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(WideStringPrint, kCodeNormalParm, "", "__print", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(WideStringSubStr, kCodeNormalParm, "start|size", "substr", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(WideStringToByte, kCodeNormalParm, "", "to_byte", kTypeIdWideString, kFlagMethod));

    AddTemplate(kTypeIdNull, ObjectPlanner(NullCopy, kStrEmpty));
  }


}