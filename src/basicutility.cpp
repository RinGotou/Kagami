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
      string option = object.GetTypeId();
      CopyCreator copyCreator = nullptr;
      map<string, ObjTemplate>::iterator it = GetTemplateMap().find(option);
      if (it != GetTemplateMap().end()) {
        result = it->second.CreateObjectCopy(object.get());
      }
      return result;
    }

    ObjTemplate *GetTemplate(string name) {
      ObjTemplate *result = nullptr;
      map<string, ObjTemplate>::iterator it = GetTemplateMap().find(name);
      if (it != GetTemplateMap().end()) {
        result = &(it->second);
      }
      return result;
    }

    void AddTemplate(string name, ObjTemplate temp) {
      GetTemplateMap().insert(pair<string, ObjTemplate>(name, temp));
    }

    void DisposeTemplate(string name) {
      map<string, ObjTemplate>::iterator it = GetTemplateMap().find(name);
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

      while (!GetObjectStack().empty() && count > 0) {
        object = GetObjectStack().at(count - 1).Find(sign);
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
      GetObjectStack().back().Create(sign, dat, kTypeIdRawString, *objtemp, constant);
      object = GetObjectStack().back().Find(sign);
      return object;
    }

    Object *CreateObject(string sign, shared_ptr<void> ptr, string option, bool constant = false) {
      Object *object = nullptr;
      AttrTag attrTag(type::GetTemplate(option)->GetMethods(), constant);
      GetObjectStack().back().add(sign, Object().set(ptr, option, Kit().MakeAttrTagStr(attrTag)));
      object = GetObjectStack().back().Find(sign);
      return object;
    }

    string GetTypeId(string sign) {
      string result = kTypeIdNull;
      Object *object = nullptr;
      size_t count = GetObjectStack().size();

      while (count > 0) {
        object = GetObjectStack().at(count - 1).Find(sign);
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
      Attachment attachment = nullptr;
      MemoryDeleter deleter = nullptr;
      //StrMap *targetmap = nullptr;
      this->first = name;
      this->second = h;

      attachment = (Attachment)GetProcAddress(this->second, "Attachment");
      deleter = this->getDeleter();
      if (attachment != nullptr) {
        auto ptr = attachment();
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

      GetInstanceList().push_back(Instance());
      GetInstanceList().back().Load(name, h);

      if (GetInstanceList().back().GetHealth()) {
        HINSTANCE &ins = GetInstanceList().back().second;
        vector<ActivityTemplate> temp = GetInstanceList().back().GetMap();
        castAttachment = (CastAttachment)GetProcAddress(ins, "CastAttachment");
        deleter = GetInstanceList().back().getDeleter();

        for (auto &unit : temp) {
          Inject(EntryProvider(unit));
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

      vector<Instance>::iterator instance_i = GetInstanceList().begin();
      while (instance_i != GetInstanceList().end()) {
        if (instance_i->first == name) break;
        instance_i++;
      }
      if (instance_i == GetInstanceList().end() && instance_i->first != name) {
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
          RemoveByTemplate(unit);
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
      GetInstanceList().erase(instance_i);
    }

    size_t ResetPlugin() {
      vector<Instance> &base = GetInstanceList();
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
    const string path = CastToString(p.at("path").get());
    Message result;
    Attachment attachment = nullptr;
    Activity activity = nullptr;
    std::wstring wpath = s2ws(Kit().GetRawString(path));
    
    HINSTANCE hinstance = LoadLibrary(wpath.c_str());
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

  Message GetSize(ObjectMap &p) {
    Message result;
    Object object = p.at("object");
    string type_id = object.GetTypeId();

    if (type_id == kTypeIdArrayBase) {
      result.SetDetail(to_string(static_pointer_cast<deque<Object>>(object.get())->size()));
    }
    else if (type_id == kTypeIdRawString) {
      auto str = *static_pointer_cast<string>(object.get());
      result.SetDetail(to_string(str.size()));
    }

    result.SetValue(kStrRedirect);
    return result;
  }

  Message GetElement(ObjectMap &p) {
    Message result;
    Object object = p.at("object"), subscript_1 = p.at("subscript_1");
    string type_id = object.GetTypeId();
    int size = 0;
    int count0 = 0;

    if (type_id == kTypeIdRawString) {
      count0 = stoi(*static_pointer_cast<string>(subscript_1.get()));
      size = static_pointer_cast<string>(object.get())->size();
      if (count0 <= size - 1) {
        result.combo(kStrRedirect,kCodeSuccess,string().append(1,
          static_pointer_cast<string>(object.get())->at(count0)));
      }
    }
    else if (type_id == kTypeIdArrayBase) {
      count0 = stoi(*static_pointer_cast<string>(subscript_1.get()));
      size = static_pointer_cast<deque<Object>>(object.get())->size();
      if (count0 <= size - 1) {
        auto &target = static_pointer_cast<deque<Object>>(object.get())->at(count0);
        result.combo(target.GetTypeId(), kCodeObject, "__element");
        result.GetPtr() = target.get();
      }
    }

    return result;
  }

  Message GetElement_2Dimension(ObjectMap &p) {
    Message result;
    Object object = p.at("object"), subscript_1 = p.at("subscript_1"), subscript_2 = p.at("subscript_2");
    string type_id = object.GetTypeId();
    int size = 0;
    int count0 = 0, count1 = 0;

    //TODO:

    return result;
  }

  Message VersionInfo(ObjectMap &p) {
    Message result(kStrRedirect, kCodeSuccess, "\"" + kEngineVersion + "\"");
    return result;
  }

  //TODO:rewrite this!
  Message PrintOnScreen(ObjectMap &p) {
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    string msg = CastToString(p.at("msg").get());
    if (Kit().GetDataType(msg) == kTypeString) {
      msg = msg.substr(1, msg.size() - 2);
    }
    std::cout << msg << std::endl;
    return result;
  }

  /*
  Init all basic objects and entries
  Just do not edit unless you want to change processor's basic behaviors.
  */
  void Activiate() {
    using namespace entry;
    ActivityTemplate temp;
    Inject(EntryProvider(temp.set("array", ArrayConstructor, kFlagNormalEntry, kCodeAutoFill, "size|init_value")));
    Inject(EntryProvider(temp.set("at", GetElement, kFlagMethod, kCodeNormalArgs, "object|subscript_1", kTypeIdRawString)));
    Inject(EntryProvider(temp.set("at", GetElement, kFlagMethod, kCodeNormalArgs, "object|subscript_1", kTypeIdArrayBase)));
    Inject(EntryProvider(temp.set("at", GetElement, kFlagMethod, kCodeNormalArgs, "object|subscript_1|subscript_2", kTypeIdCubeBase)));
    Inject(EntryProvider(temp.set("binexp", BinaryOperands, kFlagBinEntry, kCodeNormalArgs, "first|second|operator")));
    Inject(EntryProvider(temp.set("elif", ConditionBranch, kFlagNormalEntry, kCodeNormalArgs, "state")));
    Inject(EntryProvider(temp.set("else", ConditionLeaf, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.set("end", TailSign, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.set("if", ConditionRoot, kFlagNormalEntry, kCodeNormalArgs, "state")));
    Inject(EntryProvider(temp.set("ImportPlugin", LoadPlugin, kFlagNormalEntry, kCodeNormalArgs, "path")));
    Inject(EntryProvider(temp.set(kStrDefineCmd, CreateOperand, kFlagNormalEntry, kCodeAutoFill, "name|source")));
    Inject(EntryProvider(temp.set(kStrSetCmd, SetOperand, kFlagNormalEntry, kCodeNormalArgs, "target|source")));
    Inject(EntryProvider(temp.set("log", WriteLog, kFlagNormalEntry, kCodeNormalArgs, "data")));
    Inject(EntryProvider(temp.set("print", PrintOnScreen, kFlagNormalEntry, kCodeNormalArgs, "msg")));
    Inject(EntryProvider(temp.set("size", GetSize, kFlagMethod, kCodeNormalArgs, "object", kTypeIdRawString)));
    Inject(EntryProvider(temp.set("size", GetSize, kFlagMethod, kCodeNormalArgs, "object", kTypeIdArrayBase)));
    Inject(EntryProvider(temp.set("version", VersionInfo, kFlagNormalEntry, kCodeNormalArgs, "")));
    Inject(EntryProvider(temp.set("while", WhileCycle, kFlagNormalEntry, kCodeNormalArgs, "state")));
  }
}