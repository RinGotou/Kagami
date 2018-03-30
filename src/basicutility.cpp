//basic function,variable,plugin etc.
//This is a part of processor core,If you don't want to change
//processor's action,do not edit.
#include "basicutility.h"

namespace Entry {
  using namespace Suzu;
  deque<MemoryProvider> childbase;
  vector<Instance> InstanceList;

  StrPair *FindChild(string name, bool Reversed = false) {
    StrPair *pair = nullptr;
    size_t mem_i = childbase.size() - 1;
    if (childbase.empty()) {
      return nullptr;
    }
    if (childbase.size() == 1 || !Reversed) {
      pair = childbase.back().find(name);
    }
    else if (Reversed && childbase.size() > 1) {
      while (mem_i >= 0) {
        pair = childbase.at(mem_i).find(name);
        if (pair != nullptr) {
          break;
        }
        mem_i--;
      }
    }
    return pair;
  }

  StrPair CreateChild(string name, string value = kStrNull) {
    StrPair result(kStrNull, kStrNull);
    if (regex_match(name, kPatternFunction) == false) {
      Tracking::log(Message(kStrFatalError, kCodeIllegalArgs, "Illegal variable name"));
      return result;
    }
    result.first = name;
    result.second = value;
    childbase.back().create(result);
    return result;
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
          Inject(EntryProvider(unit, activity));
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

  void ResetPlugin() {
    HINSTANCE *hinstance = nullptr;
    while (InstanceList.empty() != true) {
      if (InstanceList.back().GetHealth()) {
        hinstance = &(InstanceList.back().second);
        FreeLibrary(*hinstance);
      }
      InstanceList.pop_back();
    }
    ResetPluginEntry();
  }
}

namespace Suzu {
  bool MemoryProvider::dispose(string name) {
    bool result = true;
    MemPtr ptr;
    if (dict.empty() == false) {
      ptr = dict.begin();
      while (ptr != dict.end() && ptr->first != name) ++ptr;
      if (ptr == dict.end() && ptr->first != name) result = false;
      else {
        dict.erase(ptr);
      }
    }
    return result;
  }

  string MemoryProvider::query(string name) {
    string result;
    StrPair *ptr = find(name);
    if (ptr != nullptr) result = ptr->second;
    else result = kStrNull;
    return result;
  }

  string MemoryProvider::set(string name, string value) {
    string result;
    StrPair *ptr = find(name);
    if (ptr != nullptr && ptr->IsReadOnly() != true) {
      result = ptr->second;
      ptr->second = value;
    }
    else {
      result = kStrNull;
    }
    return result;
  }

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
    using Entry::childbase;
    int intA = 0, intB = 0;
    double doubleA = 0.0, doubleB = 0.0;
    enum { EnumDouble, EnumInt, EnumStr, EnumNull }type = EnumNull;
    Message result(kStrRedirect,kCodeSuccess,"0");
    array<string, 3> buf;
    string temp = kStrEmpty;
    auto CheckingOr = [&](regex pat) -> bool {
      return (regex_match(res.at(0), pat) || regex_match(res.at(1), pat));
    };
    auto CheckingAnd = [&](regex pat) -> bool {
      return (regex_match(res.at(0), pat) && regex_match(res.at(1), pat));
    };

    if (res.size() < 3) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Calculating() 1");
      return result;
    }

    buf = { res.at(0),res.at(1),res.at(2) };

    //array data format rule:number number operator
    if (regex_match(buf.at(0), kPatternFunction)) {
      temp = childbase.back().query(buf.at(0));
      if (temp != kStrNull) buf.at(0) = temp;
      temp = kStrNull;
    }
    if (regex_match(buf.at(1), kPatternFunction)) {
      temp = childbase.back().query(buf.at(1));
      if (temp != kStrNull)buf.at(1) = temp;
      temp = kStrNull;
    }

    if (CheckingOr(kPatternString)) {
      type = EnumStr;
    }
    else if (CheckingOr(kPatternDouble)) {
      type = EnumDouble;
    }
    else if (CheckingAnd(kPatternInteger)) {
      type = EnumInt;
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Calculating() 3");
      return result;
    }
  
    switch (type) {
    case EnumInt: 
      intA = stoi(res.at(0));
      intB = stoi(res.at(1));
      result.SetDetail(to_string(Util().Calc(intA, intB, buf.at(2))));
      break;
    case EnumDouble:
      doubleA = stod(res.at(0));
      doubleB = stod(res.at(1));
      result.SetDetail(to_string(Util().Calc(intA, intB, buf.at(2))));
      break;
    case EnumStr:
      //it's reversed!
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

      result.SetDetail(buf.at(1) + buf.at(0));
      break;
    case EnumNull:
    default:
      result.combo(kStrFatalError, kCodeIllegalArgs, "Calculating() 4");
      break;
    }

    return result;
  }
  
  Message FindVariable2(deque<string> &res) {
    using namespace Entry;
    Message result(kStrSuccess, kCodeSuccess, kStrEmpty);
    StrPair *pairptr = FindChild(res.at(0), (res.at(1) == kStrTrue));
    if (pairptr != nullptr) {
      result.combo(kStrRedirect, kCodeSuccess, pairptr->second);
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, "Varibale " + res.at(0) + " is not found");
    }

    return result;
  }

  Message SetVariable(deque<string> &res) {
    using namespace Entry;
    Message result(kStrSuccess, kCodeSuccess, kStrEmpty);
    //string temp;
    StrPair *pairptr = FindChild(res.at(0));
    StrPair pair(res.at(0), res.at(1));

    if (regex_match(res.at(1), kPatternFunction)) {
      pairptr = FindChild(res.at(1));
      if (pairptr != nullptr) {
        pair.second = pairptr->second;
      }
      else {
        result.combo(kStrFatalError, kCodeIllegalCall, "Varibale " + res.at(0) + " is not found");
        return result;
      }
    }

    pairptr = FindChild(pair.first);
    if (pairptr != nullptr) {
      pairptr->second = pair.second;
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, pair.first + " is not found and cannot be set");
    }
    return result;
  }

  Message CreateVariable(deque<string> &res) {
    using namespace Entry;
    Message result;
    string temp = kStrEmpty;
    StrPair *pairptr = FindChild(res.at(0));

    if (pairptr == nullptr) {
      if (res.size() == 2) {
        if (regex_match(res.at(1), kPatternFunction)) {
          pairptr = FindChild(res.at(1));
          if (pairptr != nullptr) {
            temp = pairptr->second;
            CreateChild(res.at(0), temp);
          }
          else {
            result.combo(kStrFatalError, kCodeIllegalCall, "variable " + res.at(1) + " is not found");
          }
          pairptr = nullptr;
        }
        else {
          CreateChild(res.at(0), res.at(1));
        }
      }
      else if (res.size() == 1) {
        CreateChild(res.at(0), kStrNull);
      }
    }
    else {
      result.combo(kStrFatalError, kCodeIllegalCall, res.at(0) + "is defined");
    }

    return result;
  }

  Message LoadPlugin(deque<string> &res) {
    using namespace Entry;
    Message result;
    Attachment attachment = nullptr;
    Activity activity = nullptr;
    StrPair pair(Util().GetRawString(res.at(0)), Util().GetRawString(res.at(1)));
    
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
    UnloadInstance(res.at(0));
    return result;
  }

  void TotalInjection() {
    using namespace Entry;
    //set root memory provider
    childbase.push_back(MemoryProvider());
    childbase.back().SetParent(&(childbase.back()));
    //inject basic Entry provider
    Inject(EntryProvider("commaexp", CommaExpression, kFlagAutoSize));
    Inject(EntryProvider("vfind", FindVariable2, 2, kFlagCoreEntry));
    Inject(EntryProvider("binexp", BinaryExp, 3, kFlagBinEntry));
    Inject(EntryProvider("log", LogPrint, 1));
    Inject(EntryProvider("import", LoadPlugin, 2));
    Inject(EntryProvider("release", UnloadPlugin, 1));
    Inject(EntryProvider("var",CreateVariable,kFlagAutoSize));
    Inject(EntryProvider("set", SetVariable, 2));

  }
}