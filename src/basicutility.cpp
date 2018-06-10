//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Suzu Nakamura
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//*Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
      const auto option = object.GetTypeId();
      const auto it = GetTemplateMap().find(option);
      if (it != GetTemplateMap().end()) {
        result = it->second.CreateObjectCopy(object.Get());
      }
      return result;
    }

    ObjTemplate *GetTemplate(const string name) {
      ObjTemplate *result = nullptr;
      const auto it = GetTemplateMap().find(name);
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
    vector<Instance> &GetInstanceList() {
      static vector<Instance> base;
      return base;
    }

    vector<ObjectManager> &GetObjectStack() {
      static vector<ObjectManager> base;
      return base;
    }

    Object *FindObject(string sign) {
      Object *object = nullptr;
      size_t count = GetObjectStack().size();
      vector<ObjectManager> &base = GetObjectStack();

      while (!base.empty() && count > 0) {
        object = base[count - 1].Find(sign);
        if (object != nullptr) {
          break;
        }
        count--;
      }
      return object;
    }

    Object *FindObjectInCurrentManager(string sign) {
      Object *object = nullptr;
      auto &base = GetObjectStack().back();

      while(!base.Empty()) {
        object = base.Find(sign);
        if(object != nullptr) {
          break;
        }
      }

      return object;
    }

    Object *CreateObject(string sign, Object &object) {
      auto &base = GetObjectStack();
      base.back().Add(sign, object);
      const auto result = base.back().Find(sign);
      return result;
    }

    string GetTypeId(const string sign) {
      auto result = kTypeIdNull;
      auto count = GetObjectStack().size();
      auto& base = GetObjectStack(); 

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
      GetObjectStack().push_back(ObjectManager());
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
      const auto deleter = this->GetDeleter();
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
      HINSTANCE *hinstance = nullptr;
      map<string, ObjTemplate> *objTemp = nullptr;
      auto &instanceList = GetInstanceList();

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
        hinstance = &(instanceI->second);
        const auto castAttachment = instanceI->GetObjTemplate();
        const auto deleter = instanceI->GetDeleter();
        //delete entries
        auto actTemp = instanceI->GetMap();
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
      auto &base = GetInstanceList();
      size_t count = 0;
      while (!base.empty()) {
        UnloadInstance(base.back().first);
        count++;
      }
      return count;
    }
#else
    //Linux Version
#endif

    //from MSDN
    // ReSharper disable CppInconsistentNaming
    std::wstring s2ws(const std::string& s) {
      const auto slength = static_cast<int>(s.length()) + 1;
      const auto len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, nullptr, 0);
      auto *buf = new wchar_t[len];
      MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
      std::wstring r(buf);
      delete[] buf;
      return r;
    }
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
    Kit kit;
    Message result(kStrRedirect, kCodeSuccess, "0");
    string temp, dataOP;
    auto first = p["first"], second = p["second"], op = p[kStrOperator];
    auto tempresult = false;
    enum { enum_int, enum_double, enum_str, enum_null } enumtype = enum_null;

    if (op.Get() != nullptr) dataOP = *static_pointer_cast<string>(op.Get());

    if (first.GetTypeId() == kTypeIdRawString && second.GetTypeId() == kTypeIdRawString) {
      auto dataA = *static_pointer_cast<string>(first.Get());
      auto dataB = *static_pointer_cast<string>(second.Get());
      const auto datatypeA = first.GetTokenType();
      const auto datatypeB = second.GetTokenType();
      if (datatypeA == kTypeDouble || datatypeB == kTypeDouble) enumtype = enum_double;
      if (datatypeA == kTypeInteger && datatypeB == kTypeInteger) enumtype = enum_int;
      if (kit.IsString(dataA) || kit.IsString(dataB)) enumtype = enum_str;

      if (enumtype == enum_int || enumtype == enum_double) {
        if (dataOP == "+" || dataOP == "-" || dataOP == "*" || dataOP == "/") {
          switch (enumtype) {
          case enum_int:temp = to_string(kit.Calc(stoi(dataA), stoi(dataB), dataOP)); break;
          case enum_double:temp = to_string(kit.Calc(stod(dataA), stod(dataB), dataOP)); break;
          default: ;
          }
        }
        else if (dataOP == "==" || dataOP == ">=" || dataOP == "<=" || dataOP == "!="
          || dataOP == "<" || dataOP == ">") {
          switch (enumtype) {
          case enum_int:tempresult = kit.Logic(stoi(dataA), stoi(dataB), dataOP); break;
          case enum_double:tempresult = kit.Logic(stod(dataA), stod(dataB), dataOP); break;
          default: ;
          }
          switch (tempresult) {
          case true:temp = kStrTrue; break;
          case false:temp = kStrFalse; break;
          }
        }
      }
      else if (enumtype == enum_str) {
        if (dataOP == "+") {
          if (dataA.back() == '\'') {
            temp = dataA.substr(0, dataA.size() - 1);
            dataA = temp;
          }
          if (dataB.front() == '\'') {
            temp = dataB.substr(1, dataB.size() - 1);
            dataB = temp;
          }
          if (dataB.back() != '\'') {
            dataB.append(1, '\'');
          }
          temp = dataA + dataB;
        }
        else if (dataOP == "!=" || dataOP == "==") {
          tempresult = kit.Logic(dataA, dataB, dataOP);
          switch (tempresult) {
          case true:temp = kStrTrue; break;
          case false:temp = kStrFalse; break;
          }
        }
        else if (dataOP == ">=" || dataOP == "<=") {
          //TODO:add in Kit::Logic()

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

  Message ReturnSign(ObjectMap &p) {
    //TODO:return specific value
    return Message();
  }

  Message LeftSelfIncrease(ObjectMap &p) {
    auto object = p["object"];
    

    return Message();
  }

  Message RightSelfIncrease(ObjectMap &p) {
    auto object = p["object"];
    

    return Message();
  }

  Message LeftSelfDecrease(ObjectMap &p) {
    auto object = p["object"];


    return Message();
  }

  Message RightSelfDecrease(ObjectMap &p) {
    auto object = p["object"];


    return Message();
  }

  Message SetOperand(ObjectMap &p) {
    Message result;
    Object source = p["source"], target = p["target"];
    const auto ptr = type::GetObjectCopy(source);

    if (!target.IsRo()) {
      auto typeId = source.GetTypeId();
      auto tokenType = source.GetTokenType();
      auto methods = source.GetMethods();

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

    const auto hinstance = LoadLibrary(wpath.c_str());
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
    Message result(kStrRedirect, kCodeSuccess, "'" + kEngineVersion + "'");
    return result;
  }

  Message Print(ObjectMap &p) {
    Kit kit;
    Message result;
    auto object = p["object"];

    if (kit.FindInStringVector("__print", object.GetMethods())) {
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
    ctime_s(nowTime, sizeof(nowTime), &now);
    return Message(kStrRedirect, kCodeSuccess, "'" + string(nowTime) + "'");
  }

  /*
  Init all basic objects and entries
  Just do not edit unless you want to change processor's basic behaviors.
  */
  void Activiate() {
    using namespace entry;
    InitTemplates();
    InitMethods();
    ActivityTemplate temp;
    Inject(EntryProvider(temp.Set("binexp", BinaryOperands, kFlagOperatorEntry, kCodeNormalArgs, "first|second")));
    Inject(EntryProvider(temp.Set("elif", ConditionBranch, kFlagNormalEntry, kCodeNormalArgs, "state")));
    Inject(EntryProvider(temp.Set("else", ConditionLeaf, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.Set("end", TailSign, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.Set("if", ConditionRoot, kFlagNormalEntry, kCodeNormalArgs, "state")));
#if defined(_WIN32)
    Inject(EntryProvider(temp.Set("ImportPlugin", LoadPlugin, kFlagNormalEntry, kCodeNormalArgs, "path")));
#else
    //Linux Version
#endif
    Inject(EntryProvider(temp.Set(kStrVar, CreateOperand, kFlagNormalEntry, kCodeAutoFill, "%name|source")));
    Inject(EntryProvider(temp.Set(kStrSet, SetOperand, kFlagNormalEntry, kCodeAutoFill, "&target|source")));
    Inject(EntryProvider(temp.Set("log", WriteLog, kFlagNormalEntry, kCodeNormalArgs, "data")));
    Inject(EntryProvider(temp.Set("print", Print, kFlagNormalEntry, kCodeNormalArgs, "object")));
    Inject(EntryProvider(temp.Set("time", TimeReport, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.Set("version", VersionInfo, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.Set("while", WhileCycle, kFlagNormalEntry, kCodeNormalArgs, "state")));
  }
}