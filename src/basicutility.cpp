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
//#define _DISABLE_TYPE_SYSTEM_

namespace kagami {
  namespace type {
    map<string, ObjTemplate> TemplateMap;

    shared_ptr<void> GetObjectCopy(Object &object) {
      shared_ptr<void> result = nullptr;
      string option = object.GetTypeId();
      CopyCreator copyCreator = nullptr;
      map<string, ObjTemplate>::iterator it = TemplateMap.find(option);
      if (it != TemplateMap.end()) {
        result = it->second.CreateObjectCopy(object.get());
      }
      return result;
    }

    ObjTemplate *GetTemplate(string name) {
      ObjTemplate *result = nullptr;
      map<string, ObjTemplate>::iterator it = TemplateMap.find(name);
      if (it != TemplateMap.end()) {
        result = &(it->second);
      }
      return result;
    }

    void AddTemplate(string name, ObjTemplate temp) {
      TemplateMap.insert(pair<string, ObjTemplate>(name, temp));
    }

    void DisposeTemplate(string name) {
      map<string, ObjTemplate>::iterator it = TemplateMap.find(name);
      if (it != TemplateMap.end()) TemplateMap.erase(it);
    }
  }

  namespace entry {
    vector<Instance> InstanceList;
    vector<ObjectManager> ObjectStack;

    Object *FindObject(string sign) {
      Object *object = nullptr;
      size_t count = ObjectStack.size();

      while (!ObjectStack.empty() && count > 0) {
        object = ObjectStack.at(count - 1).Find(sign);
        if (object != nullptr) {
          break;
        }
        count--;
      }
      return object;
    }

    Object *CreateObject(string sign, string dat, bool constant = false) {
      Object *object = nullptr;
      ObjTemplate *objtemp = type::GetTemplate(kTypeIdRawString);
      ObjectStack.back().Create(sign, dat, kTypeIdRawString, *objtemp, constant);
      object = ObjectStack.back().Find(sign);
      return object;
    }

    Object *CreateObject(string sign, shared_ptr<void> ptr, string option, bool constant = false) {
      Object *object = nullptr;
      AttrTag attrTag(type::GetTemplate(option)->GetMethods(), constant);
      ObjectStack.back().add(sign, Object().set(ptr, option, Kit().MakeAttrTagStr(attrTag)));
      object = ObjectStack.back().Find(sign);
      return object;
    }

    string GetTypeId(string sign) {
      string result = kTypeIdNull;
      Object *object = nullptr;
      size_t count = ObjectStack.size();

      while (count > 0) {
        object = ObjectStack.at(count - 1).Find(sign);
        if (object != nullptr) {
          result = object->GetTypeId();
        }
      }

      return result;
    }

    void ResetObject() {
      while (!ObjectStack.empty()) ObjectStack.pop_back();
    }

    ObjectManager &CreateManager() {
      ObjectStack.push_back(ObjectManager());
      return ObjectStack.back();
    }

    bool DisposeManager() {
      if (!ObjectStack.empty()) { ObjectStack.pop_back(); }
      return ObjectStack.empty();
    }

    bool Instance::Load(string name, HINSTANCE h) {
      Attachment attachment = nullptr;
      //StrMap *targetmap = nullptr;
      this->first = name;
      this->second = h;

      attachment = (Attachment)GetProcAddress(this->second, "Attachment");
      if (attachment != nullptr) {
        auto ptr = attachment();
        act_temp = *ptr;
        delete(ptr);
        health = true;
      }
      else {
        health = false;
      }

      return health;
    }

    //from MSDN
    std::wstring s2ws(const std::string& s) {
      int len;
      int slength = (int)s.length() + 1;
      len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
      wchar_t *buf = new wchar_t[len];
      MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
      std::wstring r(buf);
      delete[] buf;
      return r;
    }

    void AddInstance(string name, HINSTANCE h) {
      //PluginActivity activity = nullptr;
      CastAttachment castAttachment = nullptr;
      map<string, ObjTemplate> *objtemps = nullptr;
      vector<ActivityTemplate> *activities = nullptr;
      MemoryDeleter deleter = nullptr;

      InstanceList.push_back(Instance());
      InstanceList.back().Load(name, h);

      if (InstanceList.back().GetHealth()) {
        HINSTANCE &ins = InstanceList.back().second;
        vector<ActivityTemplate> temp = InstanceList.back().GetMap();
        castAttachment = (CastAttachment)GetProcAddress(ins, "CastAttachment");
        deleter = InstanceList.back().getDeleter();

        for (auto &unit : temp) {
          Inject(unit.id, EntryProvider(unit));
        }
        if (castAttachment != nullptr) {
          objtemps = castAttachment();
          for (auto &unit : *objtemps) {
            type::AddTemplate(unit.first, unit.second);
          }
        }
      }
    }

    void UnloadInstance(string name) {
      Attachment attachment = nullptr;
      CastAttachment castAttachment = nullptr;
      MemoryDeleter deleter = nullptr;
      HINSTANCE *hinstance = nullptr;
      map<string, ObjTemplate> *obj_temp = nullptr;
      vector<ActivityTemplate> act_temp;
      //StrMap map;

      vector<Instance>::iterator instance_i = InstanceList.begin();
      while (instance_i != InstanceList.end()) {
        if (instance_i->first == name) break;
        instance_i++;
      }
      if (instance_i == InstanceList.end() && instance_i->first != name) {
        trace::log(Message(kStrWarning, kCodeIllegalCall, "Instance is not found, is it loaded?"));
        return;
      }

      if (instance_i->GetHealth() == true) {
        hinstance = &(instance_i->second);
        castAttachment = instance_i->getObjTemplate();
        deleter = instance_i->getDeleter();
        //delete entries
        act_temp = instance_i->GetMap();
        for (auto unit : act_temp) {
          Delete(unit.id);
        }
        //delete object templates
        if (castAttachment != nullptr) {
          obj_temp = castAttachment();
          for (auto &unit : *obj_temp) {
            type::DisposeTemplate(unit.first);
          }
        }
        //delete memory
        deleter(obj_temp);
      }
      FreeLibrary(*hinstance);
      InstanceList.erase(instance_i);
    }

    void ResetPlugin(bool OnExit) {
      HINSTANCE *hinstance = nullptr;
      while (InstanceList.empty() != true) {
        if (InstanceList.back().GetHealth()) {
          hinstance = &(InstanceList.back().second);
          FreeLibrary(*hinstance);
        }
        InstanceList.pop_back();
      }
      if (!OnExit) ResetPluginEntry();
    }
  }

  Message WriteLog(ObjectMap &p) {
    Kit kit;
    Message result;
    Object data = p.at("data");
    ofstream ofs("script.log", std::ios::out | std::ios::app);

    if (data.GetTypeId() == kTypeIdRawString) {
      auto ptr = static_pointer_cast<string>(data.get());
      if (kit.GetDataType(*ptr) == kTypeString) {
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
    Object first = p.at("first"), second = p.at("second"), op = p.at("operator");
    string temp = kStrEmpty, dataOP = kStrEmpty, dataA = kStrEmpty, dataB = kStrEmpty;
    bool tempresult = false, health = true;
    size_t count = 0;
    enum { EnumInt, EnumDouble, EnumStr, EnumNull } enumtype = EnumNull;
    int datatypeA = kTypeNull, datatypeB = kTypeNull;

    if (op.get() != nullptr) dataOP = *static_pointer_cast<string>(op.get());

    if (first.GetTypeId() == kTypeIdRawString && second.GetTypeId() == kTypeIdRawString) {
      dataA = *static_pointer_cast<string>(first.get()),
      dataB = *static_pointer_cast<string>(second.get());
      datatypeA = kit.GetDataType(dataA);
      datatypeB = kit.GetDataType(dataB);
      if (datatypeA == kTypeDouble || datatypeB == kTypeDouble) enumtype = EnumDouble;
      if (datatypeA == kTypeInteger && datatypeB == kTypeInteger) enumtype = EnumInt;
      if (datatypeA == kTypeString || datatypeB == kTypeString) enumtype = EnumStr;

      if (enumtype == EnumInt || enumtype == EnumDouble) {
        if (dataOP == "+" || dataOP == "-" || dataOP == "*" || dataOP == "/") {
          switch (enumtype) {
          case EnumInt:temp = to_string(kit.Calc(stoi(dataA), stoi(dataB), dataOP)); break;
          case EnumDouble:temp = to_string(kit.Calc(stod(dataA), stod(dataB), dataOP)); break;
          }
        }
        else if (dataOP == "==" || dataOP == ">=" || dataOP == "<=" || dataOP == "!=") {
          switch (enumtype) {
          case EnumInt:tempresult = kit.Logic(stoi(dataA), stoi(dataB), dataOP); break;
          case EnumDouble:tempresult = kit.Logic(stod(dataA), stod(dataB), dataOP); break;
          }
          switch (tempresult) {
          case true:temp = kStrTrue; break;
          case false:temp = kStrFalse; break;
          }
        }
        else if (enumtype == EnumStr) {
          if (dataOP == "+") {
            if (dataB.back() == '"') {
              temp = dataB.substr(0, dataB.size() - 1);
              dataB = temp;
              temp = kStrEmpty;
            }
            if (dataA.front() == '"') {
              temp = dataA.substr(1, dataA.size() - 1);
              dataA = temp;
              temp = kStrEmpty;
            }
          }
          else if (dataOP == "!=" || dataOP == "==") {
            tempresult = kit.Logic(dataB, dataA, dataOP);
            switch (tempresult) {
            case true:temp = kStrTrue; break;
            case false:temp = kStrFalse; break;
            }
          }
          else if (dataOP == ">=" || dataOP == "<=") {
            //TODO:add in Kit::Logic()
          }
        }
      }
      else {
        //TODO:other type
      }
    }
    if (health) {
      result.SetDetail(temp);
    }
    return result;
  }

  Message ConditionRoot(ObjectMap &p) {
    auto object = p.at("state");
    Message result(CastToString(object.get()), kCodeConditionRoot, kStrEmpty);
    return result;
  }

  Message ConditionBranch(ObjectMap &p) {
    auto object = p.at("state");
    Message result(CastToString(object.get()), kCodeConditionBranch, kStrEmpty);
    return result;
  }

  Message ConditionLeaf(ObjectMap &p) {
    Message result(kStrTrue, kCodeConditionLeaf, kStrEmpty);
    return result;
  }

  Message WhileCycle(ObjectMap &p) {
    auto object = p.at("state");
    Message result(CastToString(object.get()), kCodeHeadSign, kStrEmpty);
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
    AttrTag attrTag;
    Message result;
    Object source = p.at("source"), target = p.at("target");
    auto ptr = type::GetObjectCopy(target);

    attrTag.methods = type::GetTemplate(target.GetTypeId())->GetMethods();
    attrTag.ro = false;
    source.set(ptr, target.GetTypeId(), Kit().MakeAttrTagStr(attrTag));

    return result;

  }

  Message CreateOperand(ObjectMap &p) {
    Message result;
    Object name = p.at("name"), source = p.at("source");
    string name_value = CastToString(name.get());
    Object *ptr = entry::FindObject(name_value);
    shared_ptr<void> target_ptr = nullptr;

    if (ptr == nullptr) {
      target_ptr = type::GetObjectCopy(source);
      auto object = entry::CreateObject(CastToString(name.get()), target_ptr, source.GetTypeId(), false);
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
    //TODO:type checking
    //temp fix
    const string name = CastToString(p.at("name").get());
    const string path = CastToString(p.at("path").get());
    Message result;
    Attachment attachment = nullptr;
    Activity activity = nullptr;
    std::wstring wpath = s2ws(Kit().GetRawString(path));
    
    HINSTANCE hinstance = LoadLibrary(wpath.c_str());
    if (hinstance != nullptr) {
      AddInstance(name, hinstance);
    }
    else {
      result.combo(kStrWarning, kCodeIllegalCall, "Plugin not found.");
    }
    return result;
  }
#else
  //Linux Version
#endif

  Message ArrayConstructor(ObjectMap &p) {
    Message result;
    AttrTag attrTag("", false);
    Object size = p.at("size"), init_value = p.at("init_value");
    int size_value = stoi(*static_pointer_cast<string>(size.get()));
    int count = 0;
    shared_ptr<void> init_ptr = nullptr;
    deque<Object> base;

    //error:wrong size
    if (size_value <= 0) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Illegal array size.");
      return result;
    }

    attrTag = init_value.getTag();
    attrTag.ro = false;

    for (count = 0; count < size_value; count++) {
      init_ptr = type::GetObjectCopy(init_value);
      base.push_back(Object().set(init_ptr, init_value.GetTypeId(), Kit().MakeAttrTagStr(attrTag)));
    }

    result.combo(kTypeIdArrayBase, kCodeObject, "__result");
    result.GetPtr() = make_shared<deque<Object>>(base);

    return result;
  }

  Message GetElement(ObjectMap &p) {
    Message result;

    return result;
  }

  Message VersionInfo(ObjectMap &p) {
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    std::cout << kEngineVersion << std::endl;
    return result;
  }

  Message PrintOnScreen(ObjectMap &p) {
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    string msg = CastToString(p.at("msg").get());
    std::cout << msg << std::endl;
    return result;
  }

  /*
  Init all basic objects and entries
  Just do not edit unless you want to change processor's basic behaviors.
  */
  void Activiate() {
    using namespace entry;
    Kit kit;
    auto Build = [&](string target) { return kit.BuildStringVector(target); };

    Inject("end", EntryProvider(ActivityTemplate()
      .set("end", TailSign, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject("while", EntryProvider(ActivityTemplate()
      .set("while", WhileCycle, kFlagNormalEntry, kCodeNormalArgs, "state")));
    Inject("binexp", EntryProvider(ActivityTemplate()
      .set("binexp", BinaryOperands, kFlagBinEntry, kCodeNormalArgs, "first|second|operator")));
    Inject("log", EntryProvider(ActivityTemplate()
      .set("log", WriteLog, kFlagNormalEntry, kCodeNormalArgs, "data")));
    Inject("import", EntryProvider(ActivityTemplate()
      .set("import", LoadPlugin, kFlagNormalEntry, kCodeNormalArgs, "name|path")));
    Inject(kStrDefineCmd, EntryProvider(ActivityTemplate()
      .set(kStrDefineCmd, CreateOperand, kFlagNormalEntry, kCodeAutoFill, "name|source")));
    Inject(kStrSetCmd, EntryProvider(ActivityTemplate()
      .set(kStrSetCmd, SetOperand, kFlagNormalEntry, kCodeNormalArgs, "target|source")));
    Inject("if", EntryProvider(ActivityTemplate()
      .set("if", ConditionRoot, kFlagNormalEntry, kCodeNormalArgs, "state")));
    Inject("elif", EntryProvider(ActivityTemplate()
      .set("elif", ConditionBranch, kFlagNormalEntry, kCodeNormalArgs, "state")));
    Inject("else", EntryProvider(ActivityTemplate()
      .set("else", ConditionLeaf, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject("array", EntryProvider(ActivityTemplate()
      .set("array", ArrayConstructor, kFlagNormalEntry, kCodeAutoFill, "name|init_value")));
    Inject("print", EntryProvider(ActivityTemplate()
      .set("print", PrintOnScreen, kFlagNormalEntry, kCodeNormalArgs, "msg")));
    //Inject("__get_element", EntryProvider("__get_element", GetElement, kFlagAutoFill, kFlagNormalEntry, Build("name|subscript_1|subscript_2")));
  }

  void InitObjectTemplates() {

  }
}