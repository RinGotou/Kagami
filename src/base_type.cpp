#include "base_type.h"


namespace kagami {
  //components

  //Common
  shared_ptr<void> FakeCopy(shared_ptr<void> target) {
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
      result = Message(kStrFatalError, kCodeIllegalParm, "Illegal array size.");
      return result;
    }

    const auto type_id = obj.GetTypeId();
    const auto methods = obj.GetMethods();
    const auto tokenTypeEnum = obj.GetTokenType();
    shared_ptr<void> initPtr;
    base.reserve(size);

    for (auto count = 0; count < size; count++) {
      initPtr = type::GetObjectCopy(obj);
      base.emplace_back((Object()
        .Set(initPtr, type_id)
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
      msg = Message(kStrFatalError, kCodeOverflow, "Subscript is out of range");
    }
    return msg;
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrObject];
    return Message(to_string(GetObjectStuff<ArrayBase>(obj).size()));
  }

  Message ArrayPrint(ObjectMap &p) {
    Message result;
    ObjectMap obj_map;

    if (p.CheckTypeId(kStrObject, kTypeIdArrayBase)) {
      auto &base = p.Get<ArrayBase>(kStrObject);
      auto ent = entry::Order("print", kTypeIdNull, -1);

      for (auto &unit : base) {
        obj_map.Input(kStrObject, unit);
        result = ent.Start(obj_map);
        obj_map.clear();
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

    if (util::IsString(data)) {
      data = util::GetRawString(data);
    }
    size = data.size();
    if (idx <= int(size - 1)) {
      result = Message(makeStrToken(data.at(idx)));
    }
    else {
      result = Message(kStrFatalError, kCodeOverflow, "Subscript is out of range");
    }
    
    return result;
  }

  Message RawStringGetSize(ObjectMap &p) {
    auto str = p.Get<string>(kStrObject);

    util::IsString(str) ?
      str = util::GetRawString(str) :
      str = str;

    return Message(to_string(str.size()));
  }

  Message RawStringPrint(ObjectMap &p) {
    Message result;
    string msg;
    bool doNotWrap = (p.Search("not_wrap"));
    
    auto data = p.Get<string>(kStrObject);

    util::IsString(data) ? 
      data = util::GetRawString(data) : 
      data = data;

    std::cout << data;
    if (!doNotWrap) std::cout << std::endl;
    
    return result;
  }

  //String
  Message StringConstructor(ObjectMap &p) {
    Object &obj = p["raw_string"];
    string type_id = obj.GetTypeId();
    Object base;
    if (type_id != kTypeIdRawString 
      && type_id != kTypeIdString
      && type_id != kTypeIdWideString) {
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
      if (util::IsString(origin)) {
        output = util::GetRawString(origin);
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

  //InStream
  Message InStreamConsturctor(ObjectMap &p) {
    string path = util::GetRawString(p.Get<string>("path"));
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
      msg = Message("");
    }

    if (ifs.good()) {
      string str;
      std::getline(ifs, str);
      Object obj;
      obj.Set(make_shared<string>(str), kTypeIdString).SetMethods(kStringMethods).SetRo(false);
      msg.SetObject(obj);
    }
    else {
      msg = Message(kStrFatalError, kCodeBadStream, "InStream is not working.");
    }

    return msg;
  }

  Message InStreamEOF(ObjectMap &p) {
    ifstream &ifs = p.Get<ifstream>(kStrObject);
    string state;

    ifs.eof() ? state = kStrTrue : state = kStrFalse;
    Message msg(state);

    return msg;
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
    Message msg;

    if (!ofs.good()) {
      return Message(kStrFalse);
    }

    if (p.CheckTypeId("str",kTypeIdRawString)) {
      string output;
      string &origin = p.Get<string>("str");

      if (util::IsString(origin)) {
        output = util::GetRawString(origin);
      }
      ofs << output;
    }
    else if (p.CheckTypeId("str",kTypeIdString)) {
      string origin = p.Get<string>("str");
      ofs << origin;
    }
    else {
      msg = Message(kStrFalse);
    }
    return msg;
  }


  //regex
  Message RegexConstructor(ObjectMap &p) {
    string pattern_string = p.Get<string>("regex");

    util::IsString(pattern_string) ? 
      pattern_string = util::GetRawString(pattern_string): 
      pattern_string = pattern_string;

    shared_ptr<regex> reg = make_shared<regex>(regex(pattern_string));
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

    util::IsString(str) ? str = util::GetRawString(str) : str = str;

    string state;

    regex_match(str, pat) ? state = kStrTrue : state = kStrFalse;

    return Message(state);
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

    if (util::IsString(origin)) output = origin.substr(1, origin.size() - 2);
    else output = origin;

    wstring wstr = s2ws(output);

    base.Set(make_shared<wstring>(wstr), kTypeIdWideString)
      .SetMethods(kWideStringMethods)
      .SetRo(false);

    Message msg;
    msg.SetObject(base);

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
    AddEntry(Entry(StringFamilyGetElement<string>, kCodeNormalParm, "index", "__at", kTypeIdString, kFlagMethod));
    AddEntry(Entry(StringFamilyPrint<string, std::ostream>, kCodeNormalParm, "", "__print", kTypeIdString, kFlagMethod));
    AddEntry(Entry(StringFamilySubStr<string>, kCodeNormalParm, "start|size", "substr", kTypeIdString, kFlagMethod));
    AddEntry(Entry(GetStringFamilySize<string>, kCodeNormalParm, "", "size", kTypeIdString, kFlagMethod));
    AddEntry(Entry(StringFamilyConverting<wstring, string>, kCodeNormalParm, "", "to_wide", kTypeIdString, kFlagMethod));

    AddTemplate(kTypeIdInStream, ObjectPlanner(FakeCopy, kInStreamMethods));
    AddEntry(Entry(InStreamConsturctor, kCodeNormalParm, "path", "instream"));
    AddEntry(Entry(InStreamGet, kCodeNormalParm, "", "get", kTypeIdInStream, kFlagMethod));
    AddEntry(Entry(StreamFamilyState<ifstream>, kCodeNormalParm, "", "good", kTypeIdInStream, kFlagMethod));
    AddEntry(Entry(InStreamEOF, kCodeNormalParm, "", "eof", kTypeIdInStream, kFlagMethod));
    AddEntry(Entry(StreamFamilyClose<ifstream>, kCodeNormalParm, "", "close", kTypeIdInStream, kFlagMethod));

    AddTemplate(kTypeIdOutStream, ObjectPlanner(FakeCopy, kOutStreamMethods));
    AddEntry(Entry(OutStreamConstructor, kCodeNormalParm, "path|mode", "outstream"));
    AddEntry(Entry(OutStreamWrite, kCodeNormalParm, "str", "write", kTypeIdOutStream, kFlagMethod));
    AddEntry(Entry(StreamFamilyState<ofstream>, kCodeNormalParm, "", "good", kTypeIdOutStream, kFlagMethod));
    AddEntry(Entry(StreamFamilyClose<ofstream>, kCodeNormalParm, "", "close", kTypeIdOutStream, kFlagMethod));

    AddTemplate(kTypeIdRegex, ObjectPlanner(FakeCopy, kTypeIdRegex));
    AddEntry(Entry(RegexConstructor, kCodeNormalParm, "regex", "regex"));
    AddEntry(Entry(RegexMatch, kCodeNormalParm, "str", "match", kTypeIdRegex, kFlagMethod));

    AddTemplate(kTypeIdWideString, ObjectPlanner(SimpleSharedPtrCopy<wstring>, kWideStringMethods));
    AddEntry(Entry(WideStringContructor, kCodeNormalParm, "raw_string", "wstring"));
    AddEntry(Entry(GetStringFamilySize<wstring>, kCodeNormalParm, "", "size", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(StringFamilyGetElement<wstring>, kCodeNormalParm, "index", "__at", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(StringFamilyPrint<wstring, std::wostream>, kCodeNormalParm, "", "__print", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(StringFamilySubStr<wstring>, kCodeNormalParm, "start|size", "substr", kTypeIdWideString, kFlagMethod));
    AddEntry(Entry(StringFamilyConverting<string, wstring>, kCodeNormalParm, "", "to_byte", kTypeIdWideString, kFlagMethod));

    AddTemplate(kTypeIdNull, ObjectPlanner(NullCopy, kStrEmpty));
  }
}