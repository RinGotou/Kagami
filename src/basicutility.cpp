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
#include "windows.h"
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
      auto it = GetTemplateMap().find(option);
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
        object = base.at(count - 1).Find(sign);
        if (object != nullptr) {
          break;
        }
        count--;
      }
      return object;
    }

    Object *CreateObject(string sign, string dat, bool constant = false) {
      const auto objtemp = type::GetTemplate(kTypeIdRawString);
      GetObjectStack().back().Create(sign, dat, kTypeIdRawString, *objtemp, constant);
      const auto object = GetObjectStack().back().Find(sign);
      return object;
    }

    Object *CreateObject(const string sign, const shared_ptr<void> ptr, const string option, const bool constant = false) {
      const Attribute attribute(type::GetTemplate(option)->GetMethods(), constant);
      GetObjectStack().back().add(sign, Object().Set(ptr, option, Kit().BuildAttrStr(attribute)));
      const auto object = GetObjectStack().back().Find(sign);
      return object;
    }

    string GetTypeId(const string sign) {
      auto result = kTypeIdNull;
      auto count = GetObjectStack().size();
      auto& base = GetObjectStack(); 

      while (count > 0) {
        const auto object = base.at(count - 1).Find(sign);
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

    bool Instance::Load(string name, HINSTANCE h) {
      this->first = name;
      this->second = h;

      const auto attachment = Attachment(GetProcAddress(this->second, "Attachment"));
      const auto deleter = this->getDeleter();
      if (attachment != nullptr) {
        const auto ptr = attachment();
        act_temp = *ptr;
        deleter(ptr);
        health = true;
      }
      else {
        health = false;
      }

      return health;
    }

    //from MSDN
    // ReSharper disable CppInconsistentNaming
    std::wstring s2ws(const std::string& s) {
      // ReSharper restore CppInconsistentNaming
      //int len;
      // ReSharper disable once CppLocalVariableMayBeConst
      auto slength = static_cast<int>(s.length()) + 1;
      // ReSharper disable once CppLocalVariableMayBeConst
      auto len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
      auto *buf = new wchar_t[len];
      MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
      std::wstring r(buf);
      delete[] buf;
      return r;
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

      auto instanceI = GetInstanceList().begin();
      while (instanceI != GetInstanceList().end()) {
        if (instanceI->first == name) break;
        ++instanceI;
      }
      if (instanceI == GetInstanceList().end() && instanceI->first != name) {
        trace::Log(Message(kStrWarning, kCodeIllegalCall, "Instance is not found, is it loaded?"));
        return;
      }

      if (instanceI->GetHealth() == true) {
        hinstance = &(instanceI->second);
        const auto castAttachment = instanceI->getObjTemplate();
        const auto deleter = instanceI->getDeleter();
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
      GetInstanceList().erase(instanceI);
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
  }

  Message WriteLog(ObjectMap &p) {
    Kit kit;
    Message result;
    Object data = p.at("data");
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
    auto first = p.at("first"), second = p.at("second"), op = p.at(kStrOperator);
    auto temp = kStrEmpty, dataOP = kStrEmpty;
    auto tempresult = false;
    enum { enum_int, enum_double, enum_str, enum_null } enumtype = enum_null;

    if (op.Get() != nullptr) dataOP = *static_pointer_cast<string>(op.Get());

    if (first.GetTypeId() == kTypeIdRawString && second.GetTypeId() == kTypeIdRawString) {
      auto dataA = *static_pointer_cast<string>(first.Get());
      auto dataB = *static_pointer_cast<string>(second.Get());
      const auto datatypeA = kit.GetDataType(dataA);
      const auto datatypeB = kit.GetDataType(dataB);
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
    auto object = p.at("state");
    Message result(CastToString(object.Get()), kCodeConditionRoot, kStrEmpty);
    return result;
  }

  Message ConditionBranch(ObjectMap &p) {
    auto object = p.at("state");
    Message result(CastToString(object.Get()), kCodeConditionBranch, kStrEmpty);
    return result;
  }

  Message ConditionLeaf(ObjectMap &p) {
    Message result(kStrTrue, kCodeConditionLeaf, kStrEmpty);
    return result;
  }

  Message WhileCycle(ObjectMap &p) {
    auto object = p.at("state");
    Message result(CastToString(object.Get()), kCodeHeadSign, kStrEmpty);
    return result;
  }

  Message TailSign(ObjectMap &p) {
    Message result(kStrEmpty, kCodeTailSign, kStrEmpty);
    return result;
  }

  Message ReturnSign(ObjectMap &p) {
    Message result;
    //TODO:return specific value
    return result;
  }

  Message SetOperand(ObjectMap &p) {
    Attribute attribute;
    Message result;
    Object source = p.at("source"), target = p.at("target");
    const auto ptr = type::GetObjectCopy(source);

    attribute.Methods = type::GetTemplate(source.GetTypeId())->GetMethods();
    attribute.Ro = false;
    const auto attrStr = Kit().BuildAttrStr(attribute);
    target.Set(ptr, source.GetTypeId(), attrStr);

    return result;
  }

  Message CreateOperand(ObjectMap &p) {
    Message result;
    auto name = p.at("name"), source = p.at("source");
    const auto nameValue = CastToString(name.Get());
    const auto ptr = entry::FindObject(nameValue);

    if (ptr == nullptr) {
      const auto targetPtr = type::GetObjectCopy(source);
      const auto object = entry::CreateObject(CastToString(name.Get()), targetPtr, source.GetTypeId(), false);
      if (object == nullptr) {
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
    const auto path = CastToString(p.at("path").Get());
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
    Message result(kStrRedirect, kCodeSuccess, "\"" + kEngineVersion + "\"");
    return result;
  }

  Message Print(ObjectMap &p) {
    Kit kit;
    Message result;
    auto object = p.at("object");
    const auto attribute = object.GetTag();

    if (kit.FindInStringVector("__print", attribute.Methods)) {
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

  /*
  Init all basic objects and entries
  Just do not edit unless you want to change processor's basic behaviors.
  */
  void Activiate() {
    using namespace entry;
    InitTemplates();
    InitMethods();
    ActivityTemplate temp;
    Inject(EntryProvider(temp.set("binexp", BinaryOperands, kFlagOperatorEntry, kCodeNormalArgs, "first|second")));
    Inject(EntryProvider(temp.set("elif", ConditionBranch, kFlagNormalEntry, kCodeNormalArgs, "state")));
    Inject(EntryProvider(temp.set("else", ConditionLeaf, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.set("end", TailSign, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.set("if", ConditionRoot, kFlagNormalEntry, kCodeNormalArgs, "state")));
    Inject(EntryProvider(temp.set("ImportPlugin", LoadPlugin, kFlagNormalEntry, kCodeNormalArgs, "path")));
    Inject(EntryProvider(temp.set(kStrVar, CreateOperand, kFlagNormalEntry, kCodeAutoFill, "%name|source")));
    Inject(EntryProvider(temp.set(kStrSet, SetOperand, kFlagNormalEntry, kCodeAutoFill, "&target|source")));
    Inject(EntryProvider(temp.set("log", WriteLog, kFlagNormalEntry, kCodeNormalArgs, "data")));
    Inject(EntryProvider(temp.set("print", Print, kFlagNormalEntry, kCodeNormalArgs, "object")));
    Inject(EntryProvider(temp.set("version", VersionInfo, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.set("while", WhileCycle, kFlagNormalEntry, kCodeNormalArgs, "state")));
  }
}