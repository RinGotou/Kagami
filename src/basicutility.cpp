//This is a part of processor core,If you don't want to change
//processor's action,do not edit.
#include "basicutility.h"

namespace Entry {
  using namespace Suzu;
  deque<MemoryProvider> childbase;
  vector<Instance> InstanceList;

  void AddInstance(string path, string name) {
    PluginActivity activity = nullptr;
    Attachment attachment = nullptr;
    vector<string> *listptr = nullptr;
    EntryProvider provider;
    InstanceList.push_back(Instance(name, path));
    if (InstanceList.back().GetHealth() == true) {
      HINSTANCE &instance = InstanceList.back().second;
      attachment = (Attachment)GetProcAddress(instance, "Attachment");
      if (attachment != nullptr) {
        listptr = attachment();
        delete(listptr);
        for (auto unit : *listptr) {
          activity = (PluginActivity)GetProcAddress(instance, unit.c_str());
          if (activity != nullptr) {
            //autosize for default
            Inject(EntryProvider(unit, activity));
          }
          else {
            Tracking::log(Message(kStrFatalError, kCodeIllegalCall, "Cannot get entry,please contact plugin's author"));
          }
        }
      }
    }
    delete(listptr);
  }

  void UnloadInstance(string name) {
    //TODO:Enum all entry name and delete from base
    //Instance *instance = nullptr;
    Attachment attachment = nullptr;
    vector<string> *listptr = nullptr;
    vector<Instance>::iterator instance_i = InstanceList.begin();
    while (instance_i != InstanceList.end()) {
      if (instance_i->first == name) break;
    }
    if (instance_i == InstanceList.end() && instance_i->first != name) {
      Tracking::log(Message(kStrWarning, kCodeIllegalCall, "Instance is not existed"));
      return;
    }

    HINSTANCE *hinstance = &(instance_i->second);
    attachment = (Attachment)GetProcAddress(*hinstance, "Attachment");
    if (attachment != nullptr) {
      listptr = attachment();
      for (auto unit : *listptr) {
        Delete(unit);
      }
    }
    InstanceList.erase(instance_i);
    delete(listptr);
  }

  void ResetPlugin() {
    while (InstanceList.empty() != true) {
      
    }
  }
}

namespace Suzu {

  Message CommaExpression(vector<string> &res) {
    Message result;
    if (res.empty()) result.SetCode(kCodeRedirect).SetValue(kStrEmpty);
    else result.SetCode(kCodeRedirect).SetValue(res.back());
    return result;
  }

  Message MemoryQuery(vector<string> &res) {
    using namespace Entry;
    Message result;
    string temp;
    size_t begin = childbase.size() - 1;
    size_t i = 0;

    if (childbase.empty()) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "MemoryQuery() 1");
    }
    else {
      if (childbase.size() == 1 || res[1] == kArgOnce) {
        temp = childbase.back().query(res[0]);
      }
      else if (childbase.size() > 1 && res[1] != kArgOnce) {
        for (i = begin; i >= 0; --i) {
          temp = childbase[i].query(res[0]);
          if (temp != kStrNull) break;
        }
      }

      if (temp != kStrNull) {
        result.SetCode(kCodeSuccess).SetValue(kStrSuccess).SetDetail(temp);
      }
      else {
        result.combo(kStrFatalError, kCodeIllegalArgs, "MemoryQuery() 2");
      }
    }

    return result;
  }

  Message LogPrint(vector<string> &res) {
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

  Message BinaryExp(vector<string> &res) {
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

    //may not need at there?
    //Setup filter on query may be better
    if (!regex_match(res.at(2), kPatternSymbol)
      || !regex_match(res.at(0), kPatternNumber)
      || !regex_match(res.at(1), kPatternNumber)) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Calculating() 2");
      return result;
    }

    //start converting
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
      result.SetDetail(temp = buf.at(0) + buf.at(1));
      break;
    case EnumNull:
    default:
      result.combo(kStrFatalError, kCodeIllegalArgs, "Calculating() 4");
      break;
    }

    return result;
  }

  Message CreateVariable(vector<string> &res) {
    Message result;

    return result;
  }

  Message LoadPlugin(vector<string> &res) {
    using namespace Entry;
    Message result;
    Attachment attachment = nullptr;
    Activity activity = nullptr;
    
    HINSTANCE *hinstance = new HINSTANCE(LoadLibrary(s2ws(res.at(0)).c_str()));
    if (*hinstance != nullptr) {
      attachment = (Attachment)GetProcAddress(*hinstance, "Attachment");
      if (attachment != nullptr) {
        FreeLibrary(*hinstance);
        delete(hinstance);
        attachment = nullptr;
        AddInstance(res.at(0), res.at(1));
      }
    }
    else {
      result.combo(kStrWarning, kCodeIllegalCall, "Plugin not found");
    }
    return result;
  }

  Message UnloadPlugin(vector<string> &res) {
    Message result;

    return result;
  }

  void TotalInjection() {
    using namespace Entry;
    //set root memory provider
    childbase.push_back(MemoryProvider());
    childbase.back().SetParent(&(childbase.back()));
    //inject basic Entry provider
    Inject(EntryProvider("commaexp", CommaExpression, kFlagAutoSize));
    Inject(EntryProvider("memquery", MemoryQuery, 2, kFlagCoreEntry));
    Inject(EntryProvider("binexp", BinaryExp, 3, kFlagBinEntry));
    Inject(EntryProvider("log", LogPrint, 1));
    Inject(EntryProvider("import", LoadPlugin, 2));
  }

  //from MSDN
  std::wstring s2ws(const std::string& s)
  {
    int len;
    int slength = (int)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    wchar_t *buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
  }
}