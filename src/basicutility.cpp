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
#include "basicutility.h"
#include "windows.h"
//#define _DISABLE_TYPE_SYSTEM_

namespace kagami {
  //TODO:new type system
#ifdef _DISABLE_TYPE_SYSTEM_
  namespace type {
    map<string, CastTo> ObjectTypeMap;
    map<string, CastToExt> ObjectTypeMapExt;

    shared_ptr<void> spToString(shared_ptr<void> ptr) {
      string temp(*static_pointer_cast<string>(ptr));
      return make_shared<string>(temp);
    }

    shared_ptr<void> spToInt(shared_ptr<void> ptr) {
      int temp = *static_pointer_cast<int>(ptr);
      return make_shared<int>(temp);
    }

    shared_ptr<void> ExternProcessing(shared_ptr<void> ptr, CastToExt castTo) {
      shared_ptr<void> result = nullptr;
      if (ptr != nullptr && castTo != nullptr) {
        result.reset(castTo(ptr));
      }
      return result;
    }

    shared_ptr<void> CastToNewPtr(Object &object) {
      shared_ptr<void> result = nullptr;
      map<string, CastTo>::iterator it = ObjectTypeMap.find(object.GetTypeId());
      map<string, CastToExt>::iterator it_ex = ObjectTypeMapExt.find(object.GetTypeId());
      if (it != ObjectTypeMap.end()) {
        result = it->second(object.get());
      }
      else if (it_ex != ObjectTypeMapExt.end()) {
        result = ExternProcessing(object.get(), it_ex->second);
      }
      return result;
    }

    void InitDefaultType() {
      ObjectTypeMap.insert(CastFunc(kTypeIdRawString, spToString));
      ObjectTypeMap.insert(CastFunc(kTypeIdInt, spToInt));
    }
  }
#else
  namespace type {
    map<string, ObjTemplate> TemplateMap;

    shared_ptr<void> GetObjectCopy(Object &object) {
      shared_ptr<void> result = nullptr;
      string option = object.GetTypeId();
      CastTo castTo = nullptr;
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
#endif

  namespace entry {
    vector<Instance> InstanceList;
    vector<ObjectManager> ObjectStack;

#if defined(_DISABLE_TYPE_SYSTEM)
    Object *FindObject(string name, bool reserved = false) {
      Object *object = nullptr;
      size_t i = ObjectStack.size();
      if ((ObjectStack.size() == 1 || !reserved) && !ObjectStack.empty()) {
        object = ObjectStack.back().Find(name);
      }
      else if (ObjectStack.size() > 1 && reserved) {
        while (i > 0 && object->get() == nullptr) {
          object = ObjectStack.at(i - 1).Find(name);
          i--;
        }
      }
      return object;
    }

    Object *CreateObjectByString(string name, string str, bool readonly = false) {
      Object *object = nullptr;
      if (Kit().GetDataType(name) != kTypeFunction) {
        trace::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name."));
        return object;
      }
      ObjectStack.back().CreateByObject(name, str, kTypeIdRawString, false);
      object = ObjectStack.back().Find(name);
      return object;
    }

    Object *CreateObjectByPointer(string name, shared_ptr<void> ptr, string castoption) {
      Object *object = nullptr;
      Object temp;
      if (Kit().GetDataType(name) != kTypeFunction) {
        trace::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name."));
        return object;
      }
      temp.set(ptr, castoption);
      ObjectStack.back().CreateByObject(name, temp, false);
      object = ObjectStack.back().Find(name);
      return object;
    }

    void DisposeObject(string name, bool reserved) {
      bool result = false;
      size_t i = ObjectStack.size();
      if (ObjectStack.size() == 1 || !reserved) {
        ObjectStack.back().dispose(name);
      }
      else if (ObjectStack.size() > 1 && reserved) {
        while (result != true && i > 0) {
          ObjectStack.at(i - 1).dispose(name);
          i--;
        }
      }
    }

    void CleanupObject() {
      while (!ObjectStack.empty()) {
        ObjectStack.pop_back();
      }
    }

    ObjectManager CreateMap() {
      ObjectStack.push_back(ObjectManager());
      return ObjectStack.back();
    }

    bool DisposeMap() {
      if (!ObjectStack.empty()) {
        ObjectStack.pop_back();
      }
      return ObjectStack.empty();
    }
#else
    Object *FindObject(string sign) {
      Object *object = nullptr;
      size_t count = ObjectStack.size();

      while (!ObjectStack.empty() && count > 0) {
        object = ObjectStack.at(count - 1).Find(sign);
        if (object != nullptr) {
          break;
        }
      }
      return object;
    }

    Object *CreateObject(string sign, string dat, bool constant = false) {
      Object *object = nullptr;
      ObjectStack.back().CreateByObject(sign, dat, kTypeIdRawString, constant);
      object = ObjectStack.back().Find(sign);
      return object;
    }

    Object *CreateObject(string sign, shared_ptr<void> ptr, string option, bool constant = false) {
      Object *object = nullptr;
      Object temp;

      temp.set(ptr, option);
      ObjectStack.back().CreateByObject(sign, temp, constant);
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
#endif

    bool Instance::Load(string name, HINSTANCE h) {
      Attachment attachment = nullptr;
      StrMap *targetmap = nullptr;
      this->first = name;
      this->second = h;

      attachment = (Attachment)GetProcAddress(this->second, "Attachment");
      if (attachment != nullptr) {
        targetmap = attachment();
        link_map = StrMap(*targetmap);
        delete(targetmap);
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
      PluginActivity activity = nullptr;
      CastAttachment c_attachment = nullptr;
      map<string, ObjTemplate> *temp_map = nullptr;
      MemoryDeleter deleter = nullptr;

      InstanceList.push_back(Instance());
      InstanceList.back().Load(name, h);

      if (InstanceList.back().GetHealth()) {
        HINSTANCE &ins = InstanceList.back().second;
        StrMap map = InstanceList.back().GetMap();
        deleter = InstanceList.back().getDeleter();

        for (auto unit : map) {
          activity = (PluginActivity)GetProcAddress(ins, unit.first.c_str());
          if (activity != nullptr) {
            Inject(unit.first, EntryProvider(unit.first, activity, Kit().BuildStringVector(unit.second), deleter));
          }
        }
        c_attachment = (CastAttachment)GetProcAddress(ins, "CastAttachment");
        if (c_attachment != nullptr) {
          temp_map = c_attachment();
          for (auto &unit : *temp_map) {
            type::AddTemplate(unit.first, unit.second);
          }
        }
      }
      deleter(c_attachment);
    }

    void UnloadInstance(string name) {
      Attachment attachment = nullptr;
      CastAttachment castAttachment = nullptr;
      MemoryDeleter deleter = nullptr;
      HINSTANCE *hinstance = nullptr;
      map<string, ObjTemplate> *temp_map = nullptr;
      StrMap map;

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
        map = instance_i->GetMap();
        for (auto unit : map) {
          Delete(unit.first);
        }
        //delete object templates
        if (castAttachment != nullptr) {
          temp_map = castAttachment();
          for (auto &unit : *temp_map) {
            type::DisposeTemplate(unit.first);
          }
        }
        //delete memory
        deleter(temp_map);
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

  Message WriteLog(PathMap &p) {
    Message result;
    string r = CastToString(p.at("data"));
    ofstream ofs("script.log", std::ios::out | std::ios::app);

    if (r.at(0) == '"' && r.back() == '"') {
      ofs << r.substr(1, r.size() - 2) << "\n";
    }
    else {
      ofs << r << "\n";
    }
      
    ofs.close();
    return result;
  }

  Message BinaryOperands(PathMap &p) {
    using namespace entry;
    Kit kit;
    Object object;
    string *opercode = nullptr;
    enum { EnumDouble, EnumInt, EnumStr, EnumNull }type = EnumNull;
    Message result(kStrRedirect,kCodeSuccess,"0");
    array<string, 3> buf = { CastToString(p["first"]),
      CastToString(p["second"]),
      CastToString(p["operator"])
    };
    string temp = kStrEmpty;
    bool tempresult = false;
    size_t i = 0;
    
    auto CheckingOr = [&](regex pat) -> bool {
      return (regex_match(buf.at(0), pat) || regex_match(buf.at(1), pat));
    };
    auto CheckingAnd = [&](regex pat) -> bool {
      return (regex_match(buf.at(0), pat) && regex_match(buf.at(1), pat));
    };
    auto CheckString = [&](string str) -> bool {
      return (str.front() == '"' && str.back() == '"');
    };

    //array data format rule:number number operator
    opercode = &(buf.at(2));

    if (CheckingOr(kPatternDouble)) type = EnumDouble;
    else if (CheckingAnd(kPatternInteger)) type = EnumInt;
    else if (CheckString(buf.at(0)) || CheckString(buf.at(1))) type = EnumStr;

    if (type == EnumInt || type == EnumDouble) {
      if (*opercode == "+" || *opercode == "-" || *opercode == "*" || *opercode == "/") {
        switch (type) {
        case EnumInt:
          temp = to_string(kit.Calc(stoi(buf.at(0)), stoi(buf.at(1)), *opercode));
          break;
        case EnumDouble:
          temp = to_string(kit.Calc(stod(buf.at(0)), stod(buf.at(1)), *opercode));
          break;
        }
      }
      else if (*opercode == "==" || *opercode == ">=" || *opercode == "<=" || *opercode == "!=") {
        switch (type) {
        case EnumInt:
          tempresult = kit.Logic(stoi(buf.at(1)), stoi(buf.at(0)), *opercode);
          break;
        case EnumDouble:
          tempresult = kit.Logic(stod(buf.at(1)), stod(buf.at(0)), *opercode);
          break;
        }
        switch (tempresult) {
        case true:
          temp = kStrTrue;
          break;
        case false:
          temp = kStrFalse;
          break;
        }
      }
    }
    else if (type == EnumStr) {
      if (*opercode == "+") {
        if (buf.at(1).back() == '"') {
          temp = buf.at(1).substr(0, buf.at(1).size() - 1);
          buf.at(1) = temp;
          temp = kStrEmpty;
        }
        if (buf.at(0).front() == '"') {
          temp = buf.at(0).substr(1, buf.at(0).size() - 1);
          buf.at(0) = temp;
          temp = kStrEmpty;
        }
        if (buf.at(1).front() != '"') {
          buf.at(1) = "\"" + buf.at(1);
        }
        if (buf.at(0).back() != '"') {
          buf.at(0) = buf.at(0) + "\"";
        }
        temp = buf.at(1) + buf.at(0);
      }
      else if (*opercode == "==" || *opercode == "!=") {
        tempresult = kit.Logic(buf.at(1), buf.at(0), *opercode);
        switch (tempresult) {
        case true:
          temp = kStrTrue;
          break;
        case false:
          temp = kStrFalse;
          break;
        }
      }
      else {
        temp = "Illegal operation symbol.";
        result.SetCode(kCodeIllegalArgs).SetDetail(kStrFatalError);
      }
    }
    else {
      temp = "Illegal data type.";
      result.SetCode(kCodeIllegalArgs).SetDetail(kStrFatalError);
    }
    result.SetDetail(temp);
    return result;
  }

  Message ConditionRoot(PathMap &p) {
    Message result(CastToString(p.at("state")), kCodeConditionRoot, kStrEmpty);
    return result;
  }

  Message ConditionBranch(PathMap &p) {
    Message result(CastToString(p.at("state")), kCodeConditionBranch, kStrEmpty);
    return result;
  }

  Message ConditionLeaf(PathMap &p) {
    Message result(kStrTrue, kCodeConditionLeaf, kStrEmpty);
    return result;
  }

  Message WhileCycle(PathMap &p) {
    Message result(CastToString(p.at("state")), kCodeHeadSign, kStrEmpty);
    return result;
  }

  Message TailSign(PathMap &p) {
    Message result(kStrEmpty, kCodeTailSign, kStrEmpty);
    return result;
  }

  Message ReturnSign(PathMap &p) {
    Message result;
    //TODO:return specific value
    return result;
  }

  Message SetOperand(PathMap &p) {
    using entry::FindObject;
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    bool source_is_object = false;
    shared_ptr<void> target = nullptr, source = nullptr;
    PathMap::iterator it;

    //check left parameter
    it = p.find("target");
    if (it != p.end()) {
      target = it->second;
    }
    else {
      it = p.find("&target");
      if (it != p.end()) target = it->second;
    }

    //check right parameter
    it = p.find("source");
    if (it != p.end()) {
      source = it->second;
    }
    else {
      it = p.find("&source");
      if (it != p.end()) {
        source_is_object = true;
        source = it->second;
      }
    }

    //set operations
    if (target != nullptr) {
      auto left = static_pointer_cast<Object>(target);
      if (source_is_object && source != nullptr) {
        auto right = type::GetObjectCopy(*static_pointer_cast<Object>(source));
        if (right != nullptr) {
          left->set(right, static_pointer_cast<Object>(source)->GetTypeId());
        }
      }
      else if (!source_is_object && source != nullptr) {
        string temp = CastToString(source);
        left->manage(temp, kTypeIdRawString);
      }
      else {
        result.combo(kStrFatalError, kCodeIllegalCall, "Left parameter is illegal.");
      }
    }

    return result;
  }

  Message CreateOperand(PathMap &p) {
    using namespace entry;
    Message result;
    bool useobject = false;
    const string name = CastToString(p.at("name"));
    shared_ptr<void> source;
    Object *object = FindObject(name);
    PathMap::iterator it = p.find("source");

    if (it == p.end()) {
      it = p.find("&source");
      if (it != p.end()) {
        source = it->second;
        useobject = true;
      }
    }
    else {
      source = it->second;
    }

    if (object == nullptr) {
      if (useobject) {
        shared_ptr<void> ptr = type::GetObjectCopy(*static_pointer_cast<Object>(source));
        if (ptr != nullptr) {
          object = CreateObject(name, ptr, static_pointer_cast<Object>(source)->GetTypeId());
          if (object == nullptr) {
            result.combo(kStrFatalError, kCodeIllegalCall, "Variable creation fail.");
          }
        }
      }
      else {
        string temp = CastToString(source);
        object = CreateObject(name, temp);
        if (object == nullptr) {
          result.combo(kStrFatalError, kCodeIllegalCall, "Variable creation fail.");
        }
      }
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, name + "is already existed.");
    }
    return result;
  }


//plugin init code for Windows/Linux
#if defined(_WIN32)
  //Windows Version
  Message LoadPlugin(PathMap &p) {
    using namespace entry;
    const string name = CastToString(p.at("name"));
    const string path = CastToString(p.at("path"));
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

  Message UnloadPlugin(PathMap &p) {
    using namespace entry;
    Message result;
    UnloadInstance(Kit().GetRawString(CastToString(p.at("name"))));
    return result;
  }
#else
  //Linux Version
#endif

  Message ArrayConstructor(PathMap &p) {
    Message result;
    int size = stoi(CastToString(p.at("size")));
    int count = 0;
    shared_ptr<void> init_value = nullptr;
    shared_ptr<void> &cast_path = result.GetCastPath();
    shared_ptr<void> temp_ptr = nullptr;
    auto it = p.find("init_value");
    Object init_object;
    string object_option = kTypeIdNull;
    bool use_object = false;
    deque<Object> temp_base;

    if (size == 0) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Illegal array size.");
      return result;
    }

    if (it == p.end()) {
      it = p.find("&init_value");
      if (it != p.end()) {
        init_value = it->second;
        use_object = true;
      }
    }
    else {
      init_value = it->second;
    }

    switch (use_object) {
    case true:
      object_option = static_pointer_cast<Object>(init_value)->GetTypeId();
      for (count = 0; count < size; count++) {
        temp_ptr = type::GetObjectCopy(*static_pointer_cast<Object>(init_value));
        temp_base.push_back(Object().set(temp_ptr, object_option));
      }
      break;
    case false:
      for (count = 0; count < size; count++) {
        init_object.manage(*static_pointer_cast<string>(init_value), kTypeIdRawString);
        temp_base.push_back(init_object);
      }
      break;
    }
    
    result.combo(kTypeIdArrayBase, kCodePoint, "__result");
    cast_path = make_shared<deque<Object>>(temp_base);

    return result;
  }

#if defined(_ENABLE_FASTRING_)
  Message CharConstructor(PathMap &p) {
    Message result;

    return result;
  }

  Message FileStreamConstructor(PathMap &p) {
    Message result;

    return result;
  }
#endif
  

  Message GetElement(PathMap &p) {
    using entry::FindObject;
    Message result;
    string name = CastToString(p.at("name"));
    Object *target = FindObject(name), *item = nullptr;
    string option = kTypeIdNull;
    size_t subscript_1 = 0, subscript_2 = 0;
    shared_ptr<void> &cast_path = result.GetCastPath();
    PathMap::iterator it;

    if (target != nullptr) {
      option = target->GetTypeId();
      it = p.find("subscript_1");
      if (it->second != nullptr) subscript_1 = stoi(CastToString(it->second));
      it = p.find("subscript_2");
      if (it->second != nullptr) subscript_2 = stoi(CastToString(it->second));

      if (option == kTypeIdArrayBase) {
        shared_ptr<deque<Object>> ptr = static_pointer_cast<deque<Object>>(target->get());
        if (ptr != nullptr && subscript_1 < ptr->size()) {
          item = &(ptr->at(subscript_1));
          result.combo(item->GetTypeId(), kCodePoint, "__result");
          cast_path = item->get();
        }
        else {
          result.combo(kStrFatalError, kCodeOverflow, "Illegal subscript.");
        }
      }
      else if (option == kTypeIdRawString) {
        shared_ptr<string> ptr = static_pointer_cast<string>(target->get());
        string data = kStrEmpty;
        switch (Kit().GetDataType(*ptr)) {
        case kTypeString:
          data = ptr->substr(1, ptr->size() - 2);
          result.combo(kStrRedirect, kCodeSuccess, string().append(1, data.at(subscript_1)));
          break;
        default:break;
        }
      }
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, "Couldn't find item.");
    }

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

    Inject("end", EntryProvider("end", TailSign, 0));
    Inject(kStrDefineCmd, EntryProvider(kStrDefineCmd, CreateOperand, kFlagAutoFill, kFlagNormalEntry, Build("name|source")));
    Inject("while", EntryProvider("while", WhileCycle, 1, kFlagNormalEntry, Build("state")));
    Inject("binexp", EntryProvider("binexp", BinaryOperands, 3, kFlagBinEntry, Build("first|second|operator")));
    Inject("log", EntryProvider("log", WriteLog, 1, kFlagNormalEntry, Build("data")));
    Inject("import", EntryProvider("import", LoadPlugin, 2, kFlagNormalEntry, Build("name|path")));
    Inject("release", EntryProvider("release", UnloadPlugin, 1, kFlagNormalEntry, Build("name")));
    Inject(kStrSetCmd, EntryProvider(kStrSetCmd, SetOperand, 2, kFlagNormalEntry, Build("target|source")));
    Inject("if", EntryProvider("if", ConditionRoot, 1, kFlagNormalEntry, Build("state")));
    Inject("elif", EntryProvider("elif", ConditionBranch, 1, kFlagNormalEntry, Build("state")));
    Inject("else", EntryProvider("else", ConditionLeaf, 0));
    Inject("array", EntryProvider("array", ArrayConstructor, kFlagAutoFill, kFlagNormalEntry, Build("size|init_value")));
    Inject("__get_element", EntryProvider("__get_element", GetElement, kFlagAutoFill, kFlagNormalEntry, Build("name|subscript_1|subscript_2")));
  }
}