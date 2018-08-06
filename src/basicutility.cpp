#include "parser.h"
#if defined(_WIN32)
#include "windows.h"
#endif
#include <iostream>

namespace kagami {
  namespace type {
    map <string, ObjTemplate> &GetTemplateMap() {
      static map<string, ObjTemplate> base;
      return base;
    }

    shared_ptr<void> GetObjectCopy(Object &object) {
      shared_ptr<void> result = nullptr;
      const auto option       = object.GetTypeId();
      const auto it           = GetTemplateMap().find(option);

      if (it != GetTemplateMap().end()) {
        result = it->second.CreateObjectCopy(object.Get());
      }
      return result;
    }

    ObjTemplate *GetTemplate(const string name) {
      ObjTemplate *result = nullptr;
      const auto it       = GetTemplateMap().find(name);

      if (it != GetTemplateMap().end()) {
        result = &(it->second);
      }
      return result;
    }

    void AddTemplate(string name, ObjTemplate temp) {
      GetTemplateMap().insert(pair<string, ObjTemplate>(name, temp));
    }

    void DisposeTemplate(const string name) {
      const auto it = GetTemplateMap().find(name);
      if (it != GetTemplateMap().end()) GetTemplateMap().erase(it);
    }
  }

  namespace entry {
#if defined(_WIN32)
    vector<Instance> &GetInstanceList() {
      static vector<Instance> base;
      return base;
    }
#endif

    list<ObjectManager> &GetObjectStack() {
      static list<ObjectManager> base;
      return base;
    }

    Object *FindObject(string sign) {
      Object *object            = nullptr;
      size_t count              = GetObjectStack().size();
      list<ObjectManager> &base = GetObjectStack();

      while (!base.empty() && count > 0) {
        object = base[count - 1].Find(sign);
        if (object != nullptr) {
          break;
        }
        count--;
      }
      return object;
    }

    ObjectManager &GetCurrentManager() {
      return GetObjectStack().back();
    }

    Object *FindObjectInCurrentManager(string sign) {
      Object *object      = nullptr;
      ObjectManager &base = GetObjectStack().back();

      while(!base.Empty()) {
        object = base.Find(sign);
        if(object != nullptr) {
          break;
        }
      }

      return object;
    }

    Object *CreateObject(string sign, Object &object) {
      ObjectManager &base = GetObjectStack().back();

      base.Add(sign, object);
      const auto result = base.Find(sign);
      return result;
    }

    string GetTypeId(const string sign) {
      auto result = kTypeIdNull;
      auto count  = GetObjectStack().size();
      auto &base  = GetObjectStack(); 

      while (count > 0) {
        const auto object = base[count - 1].Find(sign);
        if (object != nullptr) {
          result = object->GetTypeId();
        }
        count--;
      }

      return result;
    }

    void ResetObject() {
      while (!GetObjectStack().empty()) GetObjectStack().pop_back();
    }

    ObjectManager &CreateManager() {
      auto &base = GetObjectStack();
      base.push_back(ObjectManager());
      return GetObjectStack().back();
    }

    bool DisposeManager() {
      if (!GetObjectStack().empty()) { GetObjectStack().pop_back(); }
      return GetObjectStack().empty();
    } 

#if defined(_WIN32)
    bool Instance::Load(string name, HINSTANCE h) {
      this->first = name;
      this->second = h;

      const auto attachment = Attachment(GetProcAddress(this->second, "Attachment"));
      const auto deleter    = this->GetDeleter();
      if (attachment != nullptr) {
        const auto ptr = attachment();
        actTemp = *ptr;
        deleter(ptr);
        health = true;
      }
      else {
        health = false;
      }

      return health;
    }

    void AddInstance(const string name, const HINSTANCE h) {
      auto &base = GetInstanceList();
      base.push_back(Instance());
      base.back().Load(name, h);

      if (base.back().GetHealth()) {
        auto &ins = base.back().second;
        auto temp = base.back().GetMap();
        const auto castAttachment = CastAttachment(GetProcAddress(ins, "CastAttachment"));

        for (auto &unit : temp) {
          Inject(EntryProvider(unit));
        }
        if (castAttachment != nullptr) {
          const auto objtemps = castAttachment();
          for (auto &unit : *objtemps) {
            type::AddTemplate(unit.first, unit.second);
          }
        }
      }
    }

    void UnloadInstance(const string name) {
      HINSTANCE *hinstance              = nullptr;
      map<string, ObjTemplate> *objTemp = nullptr;
      auto &instanceList                = GetInstanceList();

      auto instanceI = instanceList.begin();
      while (instanceI != instanceList.end()) {
        if (instanceI->first == name) break;
        ++instanceI;
      }
      if (instanceI == instanceList.end() && instanceI->first != name) {
        trace::Log(Message(kStrWarning, kCodeIllegalCall, "Instance is not found, is it loaded?"));
        return;
      }

      if (instanceI->GetHealth() == true) {
        hinstance                 = &(instanceI->second);
        const auto castAttachment = instanceI->GetObjTemplate();
        const auto deleter        = instanceI->GetDeleter();
        //delete entries
        auto actTemp              = instanceI->GetMap();
        for (auto unit : actTemp) {
          RemoveByTemplate(unit);
        }
        //delete object templates
        if (castAttachment != nullptr) {
          objTemp = castAttachment();
          for (auto &unit : *objTemp) {
            type::DisposeTemplate(unit.first);
          }
        }
        //delete memory
        deleter(objTemp);
      }
      FreeLibrary(*hinstance);
      instanceList.erase(instanceI);
    }

    size_t ResetPlugin() {
      auto &base  = GetInstanceList();
      size_t count = 0;
      while (!base.empty()) {
        UnloadInstance(base.back().first);
        count++;
      }
      return count;
    }

    //from MSDN
    std::wstring s2ws(const std::string& s) {
      const auto slength = static_cast<int>(s.length()) + 1;
      const auto len     = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, nullptr, 0);
      auto *buf          = new wchar_t[len];
      MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
      std::wstring r(buf);
      delete[] buf;
      return r;
    }
#else
    //Linux Version
#endif
  }

  Message WriteLog(ObjectMap &p) {
    Kit kit;
    Message result;
    auto data = p["data"];
    ofstream ofs("script.log", std::ios::out | std::ios::app);

    if (data.GetTypeId() == kTypeIdRawString) {
      const auto ptr = static_pointer_cast<string>(data.Get());
      if (kit.IsString(*ptr)) {
        ofs << ptr->substr(1, ptr->size() - 2) << "\n";
      }
      else {
        ofs << *ptr << "\n";
      }
    }
    else {
      //TODO:query
    }
    ofs.close();
    return result;
  }

  Message BinaryOperands(ObjectMap &p) {
    using entry::OperatorCode;

    Kit kit;
    Message result(kStrRedirect, kCodeSuccess, "0");
    string temp, dataOP;
    auto first = p["first"], second = p["second"], op = p[kStrOperator];
    auto tempresult = false;
    enum { enum_int, enum_double, enum_str, enum_null } enumtype = enum_null;

    if (op.Get() != nullptr) dataOP = *static_pointer_cast<string>(op.Get());
    auto opCode = entry::GetOperatorCode(dataOP);

    if (first.GetTypeId() == kTypeIdRawString && second.GetTypeId() == kTypeIdRawString) {
      auto dataA = *static_pointer_cast<string>(first.Get());
      auto dataB = *static_pointer_cast<string>(second.Get());
      const auto datatypeA = first.GetTokenType();
      const auto datatypeB = second.GetTokenType();
      if (datatypeA == kTypeDouble || datatypeB == kTypeDouble) enumtype = enum_double;
      if (datatypeA == kTypeInteger && datatypeB == kTypeInteger) enumtype = enum_int;
      if (kit.IsString(dataA) || kit.IsString(dataB)) enumtype = enum_str;

      if (enumtype == enum_int || enumtype == enum_double) {
        switch (opCode) {
        case OperatorCode::ADD:
        case OperatorCode::SUB:
        case OperatorCode::MUL:
        case OperatorCode::DIV:
          switch (enumtype) {
          case enum_int:   temp = to_string(kit.Calc(stoi(dataA), stoi(dataB), dataOP)); break;
          case enum_double:temp = to_string(kit.Calc(stod(dataA), stod(dataB), dataOP)); break;
          default:;
          }
          break;
        case OperatorCode::IS:
        case OperatorCode::MORE_OR_EQUAL:
        case OperatorCode::LESS_OR_EQUAL:
        case OperatorCode::NOT_EQUAL:
        case OperatorCode::MORE:
        case OperatorCode::LESS:
          switch (enumtype) {
          case enum_int:   tempresult = kit.Logic(stoi(dataA), stoi(dataB), dataOP); break;
          case enum_double:tempresult = kit.Logic(stod(dataA), stod(dataB), dataOP); break;
          default:;
          }
          tempresult ? temp = kStrTrue : temp = kStrFalse;
          break;
        }
      }
      else if (enumtype == enum_str) {
        switch (opCode) {
        case OperatorCode::ADD:
          if (dataA.back() == '\'') {
            temp  = dataA.substr(0, dataA.size() - 1);
            dataA = temp;
          }
          if (dataB.front() == '\'') {
            temp  = dataB.substr(1, dataB.size() - 1);
            dataB = temp;
          }
          if (dataB.back() != '\'') {
            dataB.append(1, '\'');
          }
          temp = dataA + dataB;
          break;
        case OperatorCode::NOT_EQUAL:
        case OperatorCode::EQUAL:
          tempresult = kit.Logic(dataA, dataB, dataOP);
          tempresult ? temp = kStrTrue : temp = kStrFalse;
          break;
        case OperatorCode::MORE_OR_EQUAL:
        case OperatorCode::LESS_OR_EQUAL:
          //TODO:add in Kit::Logic()
          break;
        default:break;
        }
      }
      else {
        //TODO:other type
      }
    }
    result.SetDetail(temp);

    return result;
  }

  Message ConditionRoot(ObjectMap &p) {
    return Message(CastToString(p["state"].Get()), kCodeConditionRoot, kStrEmpty);
  }

  Message ConditionBranch(ObjectMap &p) {
    return Message(CastToString(p["state"].Get()), kCodeConditionBranch, kStrEmpty);
  }

  Message ConditionLeaf(ObjectMap &p) {
    return Message(kStrTrue, kCodeConditionLeaf, kStrEmpty);
  }

  Message WhileCycle(ObjectMap &p) {
    return Message(CastToString(p["state"].Get()), kCodeHeadSign, kStrEmpty);
  }

  Message TailSign(ObjectMap &p) {
    return Message(kStrEmpty, kCodeTailSign, kStrEmpty);
  }

  Message LeftSelfIncreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if(object.GetTypeId() == kTypeIdRawString) {
      const auto origin = *static_pointer_cast<string>(object.Get());
      if (object.GetTokenType() == kTypeInteger) {
        auto data = stoi(origin);
        ++data;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
      else if (object.GetTokenType() == kTypeDouble) {
        auto data = stod(origin);
        data += 1.0f;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message LeftSelfDecreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      const auto origin = *static_pointer_cast<string>(object.Get());
      if (object.GetTokenType() == kTypeInteger) {
        auto data = stoi(origin);
        --data;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
      else if (object.GetTokenType() == kTypeDouble) {
        auto data = stod(origin);
        data -= 1.0f;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message RightSelfIncreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      auto origin = *static_pointer_cast<string>(object.Get());
      result = origin;
      if (object.GetTokenType() == kTypeInteger) {
        auto data = stoi(origin);
        ++data;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
      else if (object.GetTokenType() == kTypeDouble) {
        auto data = stod(origin);
        data += 1.0f;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message RightSelfDecreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      auto origin = *static_pointer_cast<string>(object.Get());
      result = origin;
      if (object.GetTokenType() == kTypeInteger) {
        auto data = stoi(origin);
        --data;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
      else if (object.GetTokenType() == kTypeDouble) {
        auto data = stod(origin);
        data -= 1.0f;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message ForEachHead(ObjectMap &p) {
    static stack<string> subscriptStack;
    Kit kit;
    string unitName      = *static_pointer_cast<string>(p["unit"].Get());
    string codeSubscript = *static_pointer_cast<string>(p[kStrCodeSub].Get());
    string objectName    = *static_pointer_cast<string>(p["object"].Get());
    Object *object       = entry::FindObject(objectName);
    bool result;
    size_t currentSub  = 0;
    const auto methods = object->GetMethods();
    const auto typeId  = object->GetTypeId();
    Object *objUnit    = nullptr;

    if (!kit.FindInStringGroup("at",methods) && !kit.FindInStringGroup("size",methods)) {
      return Message(kStrFatalError, kCodeIllegalCall, 
        "This object isn't compatible with For-Each method.");
    }

    if (subscriptStack.empty() || subscriptStack.top() != codeSubscript) {
      subscriptStack.push(codeSubscript);
      auto &manager = entry::CreateManager();
      manager.Add(kStrSub,  Object().Manage("0", kTypeIdNull).SetPermanent(true));
      manager.Add(unitName, Object().Manage(kStrNull, kTypeIdNull).SetPermanent(true));
    }
    else if (subscriptStack.top() == codeSubscript) {
      const auto objSub = entry::FindObjectInCurrentManager(kStrSub);
      currentSub        = stoi(*static_pointer_cast<string>(objSub->Get()));
    }
    objUnit = entry::FindObjectInCurrentManager(unitName);

    auto providerAt      = entry::Order("at", typeId, 1);
    auto providerGetSize = entry::Order("size", typeId, -1);
    if (providerAt.Good() && providerGetSize.Good()) {
      ObjectMap map;
      auto subStr = to_string(currentSub);
      map.insert(Parameter(kStrObject, Object().Ref(*object)));
      Message msg = providerGetSize.Start(map);
      size_t size = stoi(msg.GetDetail());
      if (size > currentSub) {
        map.insert(Parameter("subscript_1", Object().Manage(subStr)));
        msg = providerAt.Start(map);
        if (msg.GetCode() == kCodeObject) {
          objUnit->Copy(msg.GetObj());
          result = true;
        }
        else if (msg.GetValue() == kStrRedirect) {
          objUnit->Manage(msg.GetDetail())
                  .SetMethods(type::GetTemplate(kTypeIdRawString)->GetMethods())
                  .SetTokenType(kit.GetDataType(msg.GetDetail()));
          result = true;
        }
        else {
          result = false;
        }
      }
      else {
        result = false;
      }

      kit.CleanupMap(map);
      currentSub++;
      Object *objSub = entry::FindObjectInCurrentManager(kStrSub);
      objSub->Manage(to_string(currentSub));
    }
    else {
      return Message(kStrFatalError, kCodeIllegalCall,
        "This object isn't compatible with For-Each method.");
    }

    string value;
    result ? value = kStrTrue : value = kStrFalse;
    return Message(value, kCodeHeadSign, kStrEmpty);
  }

  Message SetOperand(ObjectMap &p) {
    Message result;
    Object source = p["source"], target = p["target"];
    const auto ptr = type::GetObjectCopy(source);

    if (!target.IsRo()) {
      auto typeId    = source.GetTypeId();
      auto tokenType = source.GetTokenType();
      auto methods   = source.GetMethods();
      target.Set(ptr, typeId)
            .SetMethods(methods)
            .SetTokenType(tokenType);
    }
    return result;
  }

  Message CreateOperand(ObjectMap &p) {
    Message result;
    auto name = p["name"], source = p["source"];
    const auto nameValue = CastToString(name.Get());
    const auto ptr = entry::FindObject(nameValue);

    if (ptr == nullptr) {
      const auto targetPtr = type::GetObjectCopy(source);
      Object object;
      object.Set(targetPtr, source.GetTypeId())
            .SetMethods(source.GetMethods())
            .SetTokenType(source.GetTokenType())
            .SetRo(false);
      auto re = entry::CreateObject(CastToString(name.Get()), object);
      if (re == nullptr) {
        result.combo(kStrFatalError, kCodeIllegalCall, "Object creation fail.");
      }
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, "Object is already existed.");
    }

    return result;
  }

//plugin init code for Windows/Linux
#if defined(_WIN32)
  //Windows Version
  Message LoadPlugin(ObjectMap &p) {
    using namespace entry;
    const auto path = CastToString(p["path"].Get());
    Message result;
    auto wpath = s2ws(Kit().GetRawString(path));

#if defined(__clang__)
    auto hinstance = LoadLibrary(path.c_str());
#else
    auto hinstance = LoadLibrary(wpath.c_str());
#endif
    if (hinstance != nullptr) {
      AddInstance(path, hinstance);
    }
    else {
      result.combo(kStrWarning, kCodeIllegalCall, "Plugin not found.");
    }
    return result;
  }
#else
  //Linux Version
#endif
  Message VersionInfo(ObjectMap &p) {
    return Message(kStrRedirect, kCodeSuccess, "'" + kEngineVersion + "'");
  }

  Message PlatformInfo(ObjectMap &p) {
    return Message(kStrRedirect, kCodeSuccess, "'" + kPlatformType + "'");
  }

  Message InsideNameInfo(ObjectMap &p) {
    return Message(kStrRedirect, kCodeSuccess, "'" + kInsideName + "'");
  }

  Message Print(ObjectMap &p) {
    Kit kit;
    Message result;
    auto object = p["object"];

    if (kit.FindInStringGroup("__print", object.GetMethods())) {
      auto provider = entry::Order("__print", object.GetTypeId(), -1);
      if (provider.Good()) {
        result = provider.Start(p);
      }
      else {
        std::cout << "You can't print this object." << std::endl;
      }
    } 
    else {
      std::cout << "You can't print this object." << std::endl;
    }

    return result;
  }

  Message TimeReport(ObjectMap &p) {
    auto now = time(nullptr);
    char nowTime[30] = { ' ' };
#if defined(_WIN32)
    ctime_s(nowTime, sizeof(nowTime), &now);
    return Message(kStrRedirect, kCodeSuccess, "'" + string(nowTime) + "'");
#else
    string TimeData(ctime(&now));
    return Message(kStrRedirect, kCodeSuccess, "'" + TimeData + "'");
#endif
  }

  /*
  Init all basic objects and entries
  Just do not edit unless you want to change processor's basic behaviors.
  */
  void Activiate() {
    using T = ActivityTemplate;
    using namespace entry;
    InitTemplates();
    InitMethods();
    ActivityTemplate temp;
    Inject(T("binexp"      , BinaryOperands, kFlagOperatorEntry, kCodeNormalParm, "first|second"));
    Inject(T("elif"        , ConditionBranch, kFlagNormalEntry, kCodeNormalParm, "state"));
    Inject(T("else"        , ConditionLeaf, kFlagNormalEntry, kCodeNormalParm, ""));
    Inject(T("end"         , TailSign, kFlagNormalEntry, kCodeNormalParm, ""));
    Inject(T(kStrFor       , ForEachHead, kFlagNormalEntry, kCodeNormalParm, "%unit|%object"));
    Inject(T("if"          , ConditionRoot, kFlagNormalEntry, kCodeNormalParm, "state"));
    Inject(T(kStrVar       , CreateOperand, kFlagNormalEntry, kCodeAutoFill, "%name|source"));
    Inject(T(kStrSet       , SetOperand, kFlagNormalEntry, kCodeAutoFill, "&target|source"));
    Inject(T("log"         , WriteLog, kFlagNormalEntry, kCodeNormalParm, "data"));
    Inject(T("lSelfDec"    , LeftSelfDecreament, kFlagNormalEntry, kCodeNormalParm, "&object"));
    Inject(T("lSelfInc"    , LeftSelfIncreament, kFlagNormalEntry,kCodeNormalParm,"&object"));
    Inject(T("print"       , Print, kFlagNormalEntry, kCodeNormalParm, "object"));
    Inject(T("rSelfDec"    , RightSelfDecreament, kFlagNormalEntry,kCodeNormalParm,"&object"));
    Inject(T("rSelfInc"    , RightSelfIncreament, kFlagNormalEntry, kCodeNormalParm, "&object"));
    Inject(T("time"        , TimeReport, kFlagNormalEntry, kCodeNormalParm, ""));
    Inject(T("version"     , VersionInfo, kFlagNormalEntry, kCodeNormalParm, ""));
    Inject(T("while"       , WhileCycle, kFlagNormalEntry, kCodeNormalParm, "state"));
    Inject(T("platform"    , PlatformInfo, kFlagNormalEntry, kCodeNormalParm, ""));
    Inject(T("codename"    , InsideNameInfo, kFlagNormalEntry, kCodeNormalParm, ""));
#if defined(_WIN32)
    Inject(T("ImportPlugin", LoadPlugin, kFlagNormalEntry, kCodeNormalParm, "path"));
#else
    //Linux Version
#endif
  }
}