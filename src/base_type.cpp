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
    ArrayBase base;
    auto size = stoi(p.Get<string>("size"));
    Object obj = Object();

    if (p.Search("init_value")) {
      obj.Copy(p["init_value"]);
      obj.SetRo(false);
    }
    
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
      .SetRo(false));
    return result;
  }

  Message ArrayGetElement(ObjectMap &p) {
    ArrayBase &base = p.Get<ArrayBase>(kStrObject);
    int idx = stoi(p.Get<string>("index"));
    int size = int(base.size());
    
    Message msg;

    if (idx < size) {
      Object temp;
      temp.Ref(base[idx]);
      msg.SetObject(temp);
    }
    else {
      msg.combo(kStrFatalError, kCodeOverflow, "Subscript is out of range");
    }
    return msg;
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrObject];
    return Message(kStrRedirect, kCodeSuccess, 
      to_string(static_pointer_cast<ArrayBase>(obj.Get())->size()));
  }

  Message ArrayPrint(ObjectMap &p) {
    Message result;
    Object object = p[kStrObject];
    ObjectMap objMap;

    if (p.CheckTypeId(kStrObject, kTypeIdArrayBase)) {
      auto &base = p.Get<ArrayBase>(kStrObject);
      auto ent = entry::Order("print", kTypeIdNull, -1);
      for (auto &unit : base) {
        objMap.Input(kStrObject, unit);
        result = ent.Start(objMap);
        objMap.clear();
      }
    }
    return result;
  }
  
  //RawString
  Message RawStringGetElement(ObjectMap &p) {
    Message result;
    Object temp;
    size_t size;
    int idx = stoi(p.Get<string>("index"));

    const auto makeStrToken = [](char target)->string {
      return string().append("'").append(1, target).append("'");
    };

    string data = p.Get<string>(kStrObject);

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
    auto str = p.Get<string>(kStrObject);

    Kit::IsString(str) ?
      str = Kit::GetRawString(str) :
      str = str;

    return Message(kStrRedirect, kCodeSuccess, to_string(str.size()));
  }

  Message RawStringPrint(ObjectMap &p) {
    Message result;
    string msg;
    bool doNotWrap = (p.Search("not_wrap"));
    
    auto data = p.Get<string>(kStrObject);

    Kit::IsString(data) ? 
      data = Kit::GetRawString(data) : 
      data = data;

    std::cout << data;
    if (!doNotWrap) std::cout << std::endl;
    
    return result;
  }

  //String
  Message StringConstructor(ObjectMap &p) {
    Object &obj = p["raw_string"];
    string typeId = obj.GetTypeId();
    Object base;
    if (typeId != kTypeIdRawString 
      && typeId != kTypeIdString
      && typeId != kTypeIdWideString) {
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
    msg.SetObject(base);
    return msg;
  }

  Message StringGetSize(ObjectMap &p) {
    string &str = p.Get<string>(kStrObject);
    return Message(kStrRedirect, kCodeSuccess, to_string(str.size()));
  }

  Message StringGetElement(ObjectMap &p) {
    string &str = p.Get<string>(kStrObject);
    int idx = stoi(p.Get<string>("index"));
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
    string &str = p.Get<string>(kStrObject);
    std::cout << str << std::endl;
    return Message();
  }

  Message StringSubStr(ObjectMap &p) {
    Message msg;
    string &str = p.Get<string>(kStrObject);
    int start = stoi(p.Get<string>("start"));  
    int size = stoi(p.Get<string>("size"));
    

    if (start < 0 || size > int(str.size()) - start) {
      msg.combo(kStrFatalError, kCodeIllegalParm, "Illegal index or size.");
    }
    else {
      string output = str.substr(start, size);
      Object obj;
      obj.Set(make_shared<string>(output), kTypeIdString)
        .SetMethods(kStringMethods)
        .SetRo(true);
      msg.SetObject(obj);
    }
    return msg;
  }

  Message StringToWide(ObjectMap &p) {
    Object base;
    Message msg;
    string origin = p.Get<string>(kStrObject);
    shared_ptr<wstring> wstr = make_shared<wstring>(s2ws(origin));

    base.Set(wstr, kTypeIdWideString)
      .SetMethods(kWideStringMethods)
      .SetRo(false);

    msg.SetObject(base);
    return msg;
  }

  //InStream
  Message InStreamConsturctor(ObjectMap &p) {
    Object &objPath = p["path"];
    //TODO:support for string type
    string path = Kit::GetRawString(p.Get<string>("path"));
    shared_ptr<ifstream> ifs = 
      make_shared<ifstream>(ifstream(path.c_str(), std::ios::in));
    Message msg;
    Object obj;

    obj.Set(ifs, kTypeIdInStream)
      .SetMethods(kInStreamMethods)
      .SetRo(false);
    msg.SetObject(obj);

    return msg;
  }

  Message InStreamGet(ObjectMap &p) {
    ifstream &ifs = p.Get<ifstream>(kStrObject);
    Message msg;

    if (ifs.eof()) {
      msg.combo(kStrRedirect, kCodeSuccess, "");
    }

    if (ifs.good()) {
      string str;
      std::getline(ifs, str);
      Object obj;
      obj.Set(make_shared<string>(str), kTypeIdString).SetMethods(kStringMethods).SetRo(false);
      msg.SetObject(obj);
    }
    else {
      msg.combo(kStrFatalError, kCodeBadStream, "InStream is not working.");
    }

    return msg;
  }

  Message InStreamGood(ObjectMap &p) {
    ifstream &ifs = p.Get<ifstream>(kStrObject);
    string state;

    ifs.good() ? state = kStrTrue : state = kStrFalse;
    Message msg(kStrRedirect, kCodeSuccess, state);

    return msg;
  }

  Message InStreamEOF(ObjectMap &p) {
    ifstream &ifs = p.Get<ifstream>(kStrObject);
    string state;

    ifs.eof() ? state = kStrTrue : state = kStrFalse;
    Message msg(kStrRedirect, kCodeSuccess, state);

    return msg;
  }

  Message InStreamClose(ObjectMap &p) {
    ifstream &ifs = p.Get<ifstream>(kStrObject);

    ifs.close();

    return Message();
  }

  //OutStream
  Message OutStreamConstructor(ObjectMap &p) {
    string path = p.Get<string>("path");
    string mode = p.Get<string>("mode");

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
    msg.SetObject(obj);
    return msg;
  }

  Message OutStreamWrite(ObjectMap &p) {
    ofstream &ofs = p.Get<ofstream>(kStrObject);
    Object &objStr = p["str"];
    Message msg;

    if (!ofs.good()) {
      return Message(kStrRedirect, kCodeSuccess, kStrFalse);
    }

    if (objStr.GetTypeId() == kTypeIdRawString) {
      string output;
      string &origin = p.Get<string>("str");

      if (Kit::IsString(origin)) {
        output = Kit::GetRawString(origin);
      }
      ofs << output;
    }
    else if (objStr.GetTypeId() == kTypeIdString) {
      string origin = p.Get<string>("str");
      ofs << origin;
    }
    else {
      msg.combo(kStrRedirect, kCodeSuccess, kStrFalse);
    }
    return msg;
  }

  Message OutStreamGood(ObjectMap &p) {
    ofstream &ofs = p.Get<ofstream>(kStrObject);
    string state;

    ofs.good() ? state = kStrTrue : state = kStrFalse;
    Message msg(kStrRedirect, kCodeSuccess, state);

    return msg;
  }

  Message OutStreamClose(ObjectMap &p) {
    ofstream &ofs = p.Get<ofstream>(kStrObject);

    ofs.close();

    return Message();
  }

  //regex
  Message RegexConstructor(ObjectMap &p) {
    string regStr = p.Get<string>("regex");

    Kit::IsString(regStr) ? 
      regStr = Kit::GetRawString(regStr): 
      regStr = regStr;

    shared_ptr<regex> reg = make_shared<regex>(regex(regStr));
    Object ret;

    ret.Set(reg, kTypeIdRegex)
      .SetMethods(kRegexMethods)
      .SetRo(false);

    Message msg;
    msg.SetObject(ret);
    return msg;
  }

  Message RegexMatch(ObjectMap &p) {
    string str = p.Get<string>("str");
    auto &pat = p.Get<regex>(kStrObject);

    Kit::IsString(str) ? str = Kit::GetRawString(str) : str = str;

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
    msg.SetObject(base);

    return msg;
  }

  Message WideStringGetSize(ObjectMap &p) {
    wstring &wstr = p.Get<wstring>(kStrObject);

    return Message(kStrRedirect, kCodeSuccess, to_string(wstr.size()));
  }

  Message WideStringGetElement(ObjectMap &p) {
    wstring &wstr = p.Get<wstring>(kStrObject);
    int size = int(wstr.size());
    int idx = stoi(p.Get<string>("index"));
    Message msg;

    if (idx < size && idx >= 0) {
      wstring output;
      output.append(1, wstr[idx]);

      Object ret;

      ret.Set(make_shared<wstring>(output), kTypeIdWideString)
        .SetMethods(kWideStringMethods)
        .SetRo(false);
      msg.SetObject(ret);
    }
    else {
      msg.combo(kStrFatalError, kCodeIllegalParm, "Index out of range.");
    }
    return msg;
  }

  Message WideStringPrint(ObjectMap &p) {
    wstring &wstr = p.Get<wstring>(kStrObject);

    std::wcout << wstr << std::endl;

    return Message();
  }

  Message WideStringSubStr(ObjectMap &p) {
    wstring &wstr = p.Get<wstring>(kStrObject);
    int start = stoi(p.Get<string>("start"));
    int size = stoi(p.Get<string>("size"));
    Message msg;

    if (start < 0 || size > int(wstr.size()) - start) {
      msg.combo(kStrFatalError, kCodeIllegalParm, "Illegal index or size.");
    }
    else {
      Object ret;
      wstring output = wstr.substr(start, size);

      ret.Set(make_shared<wstring>(output), kTypeIdWideString)
        .SetMethods(kWideStringMethods)
        .SetRo(false);
      msg.SetObject(ret);
    }
    return msg;
  }

  Message WideStringToByte(ObjectMap &p) {
    wstring &wstr = p.Get<wstring>(kStrObject);
    shared_ptr<string> str = make_shared<string>(string(ws2s(wstr)));
    Object ret;

    ret.Set(str, kTypeIdString)
      .SetMethods(kStringMethods)
      .SetRo(false);

    Message msg;
    msg.SetObject(ret);

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

    AddTemplate(kTypeIdWideString, ObjectPlanner(SimpleSharedPtrCopy<wstring>, kWideStringMethods));
    AddEntry(Entry(WideStringContructor, kCodeNormalParm, "raw_string", "wstring"));
    AddEntry(Entry(WideStringGetSize, kCodeNormalParm, "", "size", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(WideStringGetElement, kCodeNormalParm, "index", "__at", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(WideStringPrint, kCodeNormalParm, "", "__print", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(WideStringSubStr, kCodeNormalParm, "start|size", "substr", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(WideStringToByte, kCodeNormalParm, "", "to_byte", kTypeIdWideString, kFlagMethod));

    AddTemplate(kTypeIdNull, ObjectPlanner(NullCopy, kStrEmpty));
  }
}