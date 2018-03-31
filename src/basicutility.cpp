//basic function,variable,plugin etc.
//This is a part of processor core,If you don't want to change
//processor's action,do not edit.
#include "basicutility.h"

namespace Entry {
  using namespace Suzu;

  vector<MemoryMapper> MemoryAdapter;
  vector<Instance> InstanceList;

  MemoryWrapper *FindWrapper(string name, bool reserved = false) {
    MemoryWrapper *wrapper = nullptr;
    size_t i = MemoryAdapter.size();
    if ((MemoryAdapter.size() == 1 || !reserved) && !MemoryAdapter.empty()) {
      wrapper = MemoryAdapter.back().find(name);
    }
    else if (MemoryAdapter.size() > 1 && reserved) {
      while (i > 0 && wrapper == nullptr) {
        wrapper = MemoryAdapter.at(i - 1).find(name);
        i--;
      }
    }
    return wrapper;
  }

  template <class Type>
  MemoryWrapper *CreateWrapper(string name, Type data, bool readonly = false) {
    MemoryWrapper *wrapper = nullptr;
    if (Util().GetDataType(name) != kTypeFunction) {
      Tracking::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name"));
      return wrapper;
    }
    MemoryAdapter.back().create(name, data, readonly);
    wrapper = MemoryAdapter.back().find(name);
    return wrapper;
  }

  MemoryWrapper CreateWrapper(string name, MemoryWrapper wrapper) {
    MemoryWrapper *wpr = nullptr;
    if (Util().GetDataType(name) != kTypeFunction) {
      Tracking::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name"));
      return wrapper;
    }
    MemoryAdapter.back().create(name, wrapper);
    wpr = MemoryAdapter.back().find(name);
    return *wpr;
  }

  void DisposeWrapper(string name, bool reserved = false) {
    bool result = false;
    size_t i = MemoryAdapter.size();
    if (MemoryAdapter.size() == 1 || !reserved) {
      MemoryAdapter.back().dispose(name);
    }
    else if (MemoryAdapter.size() > 1 && reserved) {
      while (result != true && i > 0) {
        result = MemoryAdapter.at(i - 1).dispose(name);
        i--;
      }
    }
  }
  
  void CleanupWrapper() {
    while (!MemoryAdapter.empty()) {
      MemoryAdapter.pop_back();
    }
  }

  bool Instance::Load(string name, HINSTANCE h) {
    Attachment attachment = nullptr;
    StrListPtr listptr = nullptr;
    this->first = name;
    this->second = h;

    attachment = (Attachment)GetProcAddress(this->second, "Attachment");
    if (attachment != nullptr) {
      listptr = attachment();
      entrylist = vector<string>(*listptr);
      delete(listptr);
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
    Attachment attachment = nullptr;
    StrListPtr listptr = nullptr;
    InstanceList.push_back(Instance());
    InstanceList.back().Load(name, h);
    if (InstanceList.back().GetHealth()) {
      HINSTANCE &ins = InstanceList.back().second;
      vector<string> &lst = InstanceList.back().GetList();
      for (auto unit : lst) {
        activity = (PluginActivity)GetProcAddress(ins, unit.c_str());
        if (activity != nullptr) {
          Inject(unit, EntryProvider(unit, activity));
        }
      }
    }
  }

  void UnloadInstance(string name) {
    Attachment attachment = nullptr;
    HINSTANCE *hinstance = nullptr;
    StrListPtr listptr = nullptr;

    vector<Instance>::iterator instance_i = InstanceList.begin();
    while (instance_i != InstanceList.end()) {
      if (instance_i->first == name) break;
      instance_i++;
    }
    if (instance_i == InstanceList.end() && instance_i->first != name) {
      Tracking::log(Message(kStrWarning, kCodeIllegalCall, "Instance is not found,is it loaded?"));
      return;
    }

    if (instance_i->GetHealth() == true) {
      hinstance = &(instance_i->second);
      listptr = &(instance_i->GetList());
      for (auto unit : *listptr) {
        Delete(unit);
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

namespace Suzu {
  Message CommaExpression(deque<string> &res) {
    Message result;
    if (res.empty()) result.SetCode(kCodeRedirect).SetValue(kStrEmpty);
    else result.SetCode(kCodeRedirect).SetValue(res.back());
    return result;
  }

  Message LogPrint(deque<string> &res) {
    using namespace Tracking;
    Message result;
    string r = res.at(0);
    ofstream ofs("script.log", std::ios::out | std::ios::app);

    if (r.at(0) == '"' && r.back() == '"') {
      ofs << res.at(0).substr(1, res.at(0).size() - 2) << "\n";
    }
    else {
      ofs << r << "\n";
    }
    
    ofs.close();
    return result;
  }

  Message BinaryExp(deque<string> &res) {
    using namespace Entry;
    Util util;
    string *opercode = nullptr;
    MemoryWrapper *wrapper = nullptr;
    enum { EnumDouble, EnumInt, EnumStr, EnumNull }type = EnumNull;
    Message result(kStrRedirect,kCodeSuccess,"0");
    array<string, 3> buf;
    string temp = kStrEmpty;
    bool tempresult = false;
    size_t i = 0;
    
    auto CheckingOr = [&](regex pat) -> bool {
      return (regex_match(res.at(0), pat) || regex_match(res.at(1), pat));
    };
    auto CheckingAnd = [&](regex pat) -> bool {
      return (regex_match(res.at(0), pat) && regex_match(res.at(1), pat));
    };
    auto CheckString = [&](string str) -> bool {
      return (str.front() == '"' && str.back() == '"');
    };

    //array data format rule:number number operator
    buf = { res.at(0),res.at(1),res.at(2) };
    opercode = &(buf.at(2));

    for (i = 0; i <= 1; i++) {
      if (util.GetDataType(buf.at(i)) == kTypeFunction) {
        wrapper = FindWrapper(buf.at(i));
        if (wrapper->GetString() != kStrNull) buf.at(i) = wrapper->GetString();
      }
    }

    if (CheckingOr(kPatternDouble)) type = EnumDouble;
    else if (CheckingAnd(kPatternInteger)) type = EnumInt;
    else if (CheckString(res.at(0)) || CheckString(res.at(1))) type = EnumStr;

    if (type == EnumInt || type == EnumDouble) {
      if (*opercode == "+" || *opercode == "-" || *opercode == "*" || *opercode == "/") {
        switch (type) {
        case EnumInt:
          temp = to_string(util.Calc(stoi(buf.at(0)), stoi(buf.at(1)), *opercode));
          break;
        case EnumDouble:
          temp = to_string(util.Calc(stod(buf.at(0)), stod(buf.at(1)), *opercode));
          break;
        }
      }
      else if (*opercode == "==" || *opercode == ">=" || *opercode == "<=" || *opercode == "!=") {
        switch (type) {
        case EnumInt:
          tempresult = util.Logic(stoi(buf.at(1)), stoi(buf.at(0)), *opercode);
          break;
        case EnumDouble:
          tempresult = util.Logic(stod(buf.at(1)), stod(buf.at(0)), *opercode);
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
        tempresult = util.Logic(buf.at(1), buf.at(0), *opercode);
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
        temp = "Illegal operation";
        result.SetCode(kCodeIllegalArgs).SetDetail(kStrFatalError);
      }
    }
    else {
      temp = "Illegal data type";
      result.SetCode(kCodeIllegalArgs).SetDetail(kStrFatalError);
    }
    result.SetDetail(temp);
    return result;
  }

  Message CycleExp(deque<string> &res) {
    Message result;
    int i = 0;
    string temp = kStrEmpty;
    deque<string> buf = res;

    //I'm cosidering about for and foreach now.
    if (buf.back() == kStrFor) {
      //start end step

    }
    if (buf.back() == kStrWhile) {
      if (buf.at(0) == kStrTrue) {
        result.combo(kStrTrue, kCodeHeadSign, kStrEmpty);
      }
      if (buf.at(0) == kStrFalse) {
        result.combo(kStrFalse, kCodeHeadSign, kStrEmpty);
      }
    }
    if (buf.back() == kStrEnd) {
      result.combo(kStrEmpty, kCodeTailSign, kStrEmpty);
    }
    return result;
  }

  Message FindVariable(deque<string> &res) {
    using namespace Entry;
    Message result(kStrRedirect, kCodeSuccess, kStrEmpty);
    MemoryWrapper *wrapper = FindWrapper(res.at(0), (res.at(1) == kStrTrue));
    if (wrapper != nullptr) {
      result.combo(kStrRedirect, kCodeSuccess, wrapper->GetString());
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, "Varibale " + res.at(0) + " is not found");
    }
    return result;
  }

  Message SetVariable(deque<string> &res) {
    using namespace Entry;
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    array<string, 2> buf = { res.at(0),res.at(1) };
    MemoryWrapper *wrapper0 = FindWrapper(buf.at(0), true), *wrapper1 = nullptr;
    auto OnError = [&](string s) {
      result.combo(kStrFatalError, kCodeIllegalCall, "Varibale " + s + " is not found");
    };

    if (wrapper0 == nullptr) {
      OnError(buf.at(0));
      return result;
    }
    if (Util().GetDataType(buf.at(1)) == kTypeFunction) {
      wrapper1 = FindWrapper(buf.at(1), true);
      if (wrapper1 != nullptr) {
        switch (wrapper1->GetMode()) {
        case kModeStringPtr:wrapper0->set(wrapper1->GetString()); break;
        default:wrapper0->set(wrapper1->GetString()); break;
        }
      }
      else {
        OnError(buf.at(1));
      }
    }
    else {
      wrapper0->set(buf.at(1));
    }

    return result;
  }


  Message CreateVariable(deque<string> &res) {
    using namespace Entry;
    Message result;
    MemoryWrapper *wrapper = FindWrapper(res.at(0));
    MemoryWrapper *wrapper2 = nullptr;
    string temp = kStrEmpty;
    int type;
    auto OnError = [&](string s) {
      result.combo(kStrFatalError, kCodeIllegalCall, "Varibale " + s + " is not found");
    };

    if (wrapper == nullptr) {
      if (res.size() == 2) {
        type = Util().GetDataType(res.at(1));
        if (type == kTypeFunction) {
          wrapper2 = FindWrapper(res.at(1));
          if (wrapper2 != nullptr) {
            wrapper = new MemoryWrapper(*wrapper2);
            CreateWrapper(res.at(0), *wrapper);
          }
          else {
            OnError(res.at(1));
          }
        }
        else if (type != kTypeNull) {
          CreateWrapper(res.at(0), res.at(1));
        }
      }
      else if (res.size() == 1) {
        wrapper = new MemoryWrapper(kModeAnonymus, nullptr);
        CreateWrapper(res.at(0), *wrapper);
      }
      else {
        result.combo(kStrFatalError, kCodeIllegalCall, res.at(0) + "is defined");
      }
    }
    return result;
  }

  Message LoadPlugin(deque<string> &res) {
    using namespace Entry;
    Message result;
    Attachment attachment = nullptr;
    Activity activity = nullptr;
    pair<string, string> pair(Util().GetRawString(res.at(0)), Util().GetRawString(res.at(1)));
    
    HINSTANCE hinstance = LoadLibrary(s2ws(pair.second).c_str());
    if (hinstance != nullptr) {
      AddInstance(pair.first, hinstance);
    }
    else {
      result.combo(kStrWarning, kCodeIllegalCall, "Plugin not found");
    }
    return result;
  }

  Message UnloadPlugin(deque<string> &res) {
    using namespace Entry;
    Message result;
    UnloadInstance(Util().GetRawString(res.at(0)));
    return result;
  }

  void TotalInjection() {
    using namespace Entry;
    //set root memory provider
    MemoryAdapter.push_back(MemoryMapper());
    //inject basic Entry provider
    Inject("commaexp", EntryProvider("commaexp", CommaExpression, kFlagAutoSize));
    Inject("vfind", EntryProvider("vfind", FindVariable, 2, kFlagCoreEntry));
    Inject("binexp", EntryProvider("binexp", BinaryExp, 3, kFlagBinEntry));
    Inject("cycle", EntryProvider("cycle", CycleExp, kFlagAutoSize, kFlagBinEntry));
    Inject("log", EntryProvider("log", LogPrint, 1));
    Inject("import", EntryProvider("import", LoadPlugin, 2));
    Inject("release", EntryProvider("release", UnloadPlugin, 1));
    Inject("var", EntryProvider("var",CreateVariable,kFlagAutoSize));
    Inject("set", EntryProvider("set", SetVariable, 2));
  }
}