//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Kagami Nakamura
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

namespace Cast {
  using namespace Kagami;

  map<string, CastTo> CastMap;
  map<string, CastToEx> CastMapEx;

  shared_ptr<void> spToString(shared_ptr<void> ptr) {
    string temp(*static_pointer_cast<string>(ptr));
    return make_shared<string>(temp);
  }

  shared_ptr<void> spToInt(shared_ptr<void> ptr) {
    int temp = *static_pointer_cast<int>(ptr);
    return make_shared<int>(temp);
  }

  shared_ptr<void> ExternProcessing(shared_ptr<void> ptr, CastToEx castTo) {
    shared_ptr<void> result = nullptr;
    if (ptr != nullptr && castTo != nullptr) {
      result.reset(castTo(ptr));
    }
    return result;
  }

  shared_ptr<void> CastToNewPtr(PointWrapper &wrapper) {
    shared_ptr<void> result = nullptr;
    map<string, CastTo>::iterator it = CastMap.find(wrapper.getOption());
    map<string, CastToEx>::iterator it_ex = CastMapEx.find(wrapper.getOption());
    if (it != CastMap.end()) {
      result = it->second(wrapper.get());
    }
    else if (it_ex != CastMapEx.end()) {
      result = ExternProcessing(wrapper.get(), it_ex->second);
    }
    return result;
  }

  void InitDefaultType() {
    CastMap.insert(CastFunc(kCastString, spToString));
    CastMap.insert(CastFunc(kCastInt, spToInt));
  }
}

namespace Entry {
  vector<MemoryManager> MemoryAdapter;
  vector<Instance> InstanceList;

  PointWrapper FindWrapper(string name, bool reserved = false) {
    PointWrapper wrapper;
    size_t i = MemoryAdapter.size();
    if ((MemoryAdapter.size() == 1 || !reserved) && !MemoryAdapter.empty()) {
      wrapper = MemoryAdapter.back().Find(name);
    }
    else if (MemoryAdapter.size() > 1 && reserved) {
      while (i > 0 && wrapper.get() == nullptr) {
        wrapper = MemoryAdapter.at(i - 1).Find(name);
        i--;
      }
    }
    return wrapper;
  }

  PointWrapper CreateWrapperByString(string name, string str, bool readonly = false) {
    PointWrapper wrapper;
    if (Kit().GetDataType(name) != kTypeFunction) {
      Tracking::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name."));
      return wrapper;
    }
    MemoryAdapter.back().CreateByObject(name, str, kCastString, false);
    wrapper = MemoryAdapter.back().Find(name);
    return wrapper;
  }

  PointWrapper CreateWrapperByPointer(string name, shared_ptr<void> ptr, string castoption) {
    PointWrapper wrapper, temp;
    if (Kit().GetDataType(name) != kTypeFunction) {
      Tracking::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name."));
      return wrapper;
    }
    temp.set(ptr, castoption);
    MemoryAdapter.back().CreateByWrapper(name, temp, false);
    wrapper = MemoryAdapter.back().Find(name);
    return wrapper;
  }

  void DisposeWrapper(string name, bool reserved) {
    bool result = false;
    size_t i = MemoryAdapter.size();
    if (MemoryAdapter.size() == 1 || !reserved) {
      MemoryAdapter.back().dispose(name);
    }
    else if (MemoryAdapter.size() > 1 && reserved) {
      while (result != true && i > 0) {
        MemoryAdapter.at(i - 1).dispose(name);
        i--;
      }
    }
  }
  
  void CleanupWrapper() {
    while (!MemoryAdapter.empty()) {
      MemoryAdapter.pop_back();
    }
  }

  MemoryManager CreateMap() {
    MemoryAdapter.push_back(MemoryManager());
    return MemoryAdapter.back();
  }

  bool DisposeMap() {
    if (!MemoryAdapter.empty()) {
      MemoryAdapter.pop_back();
    }
    return MemoryAdapter.empty();
  }

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
    map<string, CastToEx> *castmap = nullptr;
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
        castmap = c_attachment();
        for (auto unit : *castmap) {
          Cast::CastMapEx.insert(unit);
        }
      }
    }
    deleter(c_attachment);
  }

  void UnloadInstance(string name) {
    Attachment attachment = nullptr;
    HINSTANCE *hinstance = nullptr;
    StrMap map;

    vector<Instance>::iterator instance_i = InstanceList.begin();
    while (instance_i != InstanceList.end()) {
      if (instance_i->first == name) break;
      instance_i++;
    }
    if (instance_i == InstanceList.end() && instance_i->first != name) {
      Tracking::log(Message(kStrWarning, kCodeIllegalCall, "Instance is not found, is it loaded?"));
      return;
    }

    if (instance_i->GetHealth() == true) {
      hinstance = &(instance_i->second);
      map = instance_i->GetMap();
      for (auto unit : map) {
        Delete(unit.first);
      }
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

namespace Kagami {
  Message CommaExp(PathMap &p) {
    Message result;
    string res = CastToString(p.at(to_string(p.size() - 1)));
    switch (p.empty()) {
    case true:result.SetCode(kCodeRedirect).SetValue(kStrEmpty); break;
    case false:result.SetCode(kCodeRedirect).SetValue(res); break;
    }
    return result;
  }

  Message WriteLog(PathMap &p) {
    Message result;
    string r = CastToString(p.at("data"));
    //string r = *static_pointer_cast<string>(p.at("data"));
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
    using namespace Entry;
    Kit Kit;
    PointWrapper wrapper;
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
          temp = to_string(Kit.Calc(stoi(buf.at(0)), stoi(buf.at(1)), *opercode));
          break;
        case EnumDouble:
          temp = to_string(Kit.Calc(stod(buf.at(0)), stod(buf.at(1)), *opercode));
          break;
        }
      }
      else if (*opercode == "==" || *opercode == ">=" || *opercode == "<=" || *opercode == "!=") {
        switch (type) {
        case EnumInt:
          tempresult = Kit.Logic(stoi(buf.at(1)), stoi(buf.at(0)), *opercode);
          break;
        case EnumDouble:
          tempresult = Kit.Logic(stod(buf.at(1)), stod(buf.at(0)), *opercode);
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
        tempresult = Kit.Logic(buf.at(1), buf.at(0), *opercode);
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

    return result;
  }

  Message FindOperand(PathMap &p) {
    using namespace Entry;
    Message result(kStrRedirect, kCodeSuccess, kStrEmpty);
    const string name = CastToString(p.at("name"));
    const string reserved = CastToString(p.at("reserved"));
    PointWrapper wrapper = FindWrapper(name, reserved == kStrTrue);
    if (wrapper.get() != nullptr) {
      result.combo(wrapper.getOption(), kCodePoint, "__" + CastToString(p.at("name")));
      result.GetCastPath() = wrapper.get();
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, "Varibale " + name + " is not found");
    }
    return result;
  }

  Message SetOperand(PathMap &p) {
    using namespace Entry;

    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    bool usewrapper = false;
    const string name = CastToString(p.at("name"));
    shared_ptr<void> source = nullptr;
    PointWrapper wrapper0 = FindWrapper(name, true);
    PointWrapper wrapper1;
    PathMap::iterator it = p.find("source");

    if (it == p.end()) {
      it = p.find("&source");
      if (it != p.end()) {
        source = it->second;
        usewrapper = true;
      }
    }
    else {
      source = it->second;
    }

    if (wrapper0.get() != nullptr) {
      if (!usewrapper) {
        string temp = CastToString(source);
        wrapper0.manage(temp, kCastString);
      }
      else {
        shared_ptr<void> ptr = Cast::CastToNewPtr(*static_pointer_cast<PointWrapper>(source));
        wrapper1 = *static_pointer_cast<PointWrapper>(source);
        if (ptr != nullptr) {
          wrapper0.set(ptr, wrapper1.getOption());
        }
      }
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, name + " is not existed,check definitions");
    }

    return result;
  }

  Message CreateOperand(PathMap &p) {
    using namespace Entry;
    Message result;
    bool usewrapper = false;
    const string name = CastToString(p.at("name"));
    shared_ptr<void> source;
    PointWrapper wrapper = FindWrapper(name);
    PathMap::iterator it = p.find("source");

    if (it == p.end()) {
      it = p.find("&source");
      if (it != p.end()) {
        source = it->second;
        usewrapper = true;
      }
    }
    else {
      source = it->second;
    }

    if (wrapper.get() == nullptr) {
      if (usewrapper) {
        shared_ptr<void> ptr = Cast::CastToNewPtr(*static_pointer_cast<PointWrapper>(source));
        PointWrapper sourcewrapper = *static_pointer_cast<PointWrapper>(source);
        if (ptr != nullptr) {
          wrapper = CreateWrapperByPointer(name, ptr, sourcewrapper.getOption());
          if (wrapper.get() == nullptr) {
            result.combo(kStrFatalError, kCodeIllegalCall, "Variable creation fail");
          }
        }
      }
      else {
        string temp = CastToString(source);
        wrapper = CreateWrapperByString(name, temp);
        if (wrapper.get() == nullptr) {
          result.combo(kStrFatalError, kCodeIllegalCall, "Variable creation fail");
        }
      }
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, name + "is already existed");
    }
    return result;
  }

  Message LoadPlugin(PathMap &p) {
    using namespace Entry;
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
      result.combo(kStrWarning, kCodeIllegalCall, "Plugin not found");
    }
    return result;
  }

  Message UnloadPlugin(PathMap &p) {
    using namespace Entry;
    Message result;
    UnloadInstance(Kit().GetRawString(CastToString(p.at("name"))));
    return result;
  }

  void InjectBasicEntries() {
    using namespace Entry;
    Kit Kit;
    auto Build = [&](string target) {return Kit.BuildStringVector(target); };
    vector<string> temp = Kit.BuildStringVector("name");

    //if elif else
    Inject("end", EntryProvider("end", TailSign, 0));
    Inject("commaexp", EntryProvider("commaexp", CommaExp, kFlagAutoSize));
    Inject(kStrDefineCmd, EntryProvider(kStrDefineCmd, CreateOperand, kFlagAutoFill, kFlagNormalEntry, Build("name|source")));
    Inject("while", EntryProvider("while", WhileCycle, 1, kFlagNormalEntry, Build("state")));
    Inject("vfind", EntryProvider("vfind", FindOperand, 2, kFlagCoreEntry, Build("name|reserved")));
    Inject("binexp", EntryProvider("binexp", BinaryOperands, 3, kFlagBinEntry, Build("first|second|operator")));
    Inject("log", EntryProvider("log", WriteLog, 1, kFlagNormalEntry, Build("data")));
    Inject("import", EntryProvider("import", LoadPlugin, 2, kFlagNormalEntry, Build("name|path")));
    Inject("release", EntryProvider("release", UnloadPlugin, 1, kFlagNormalEntry, Build("name")));
    Inject(kStrSetCmd, EntryProvider(kStrSetCmd, SetOperand, 2, kFlagNormalEntry, Build("name|source")));
  }
}