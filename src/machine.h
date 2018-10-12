#pragma once

#include <fstream>
#include "trace.h"

namespace kagami {
  enum GroupTypeEnum { G_INT, G_FLOAT, G_STR, G_NUL };

  template <class T>
  T &GetObjectStuff(Object &obj) {
    return *static_pointer_cast<T>(obj.Get());
  }

  template <class T>
  shared_ptr<void> SimpleSharedPtrCopy(shared_ptr<void> target) {
    T temp(*static_pointer_cast<T>(target));
    return make_shared<T>(temp);
  }

  template <class T>
  Object MakeObject(T t) {
    string str = to_string(t);
    return Object().Manage(str)
      .SetMethods(type::GetMethods(kTypeIdRawString))
      .SetTokenType(Kit::GetTokenType(str))
      .SetRo(false);
  }

  template <class T>
  using ConvertFunc = T(*)(const string &);

  template<class T>
  string IncAndDec(Object &obj, bool negative, bool keep, T t, ConvertFunc<T> func) {
    string res;
    auto origin = GetObjectStuff<string>(obj);

    T data = func(origin);
    negative ?
      data -= t :
      data += t;
    keep ?
      res = origin :
      res = to_string(data);
    obj.Copy(MakeObject(data));

    return res;
  }

  class Meta {
    bool health;
    vector<Instruction> actionBase;
    size_t index;
    Token mainToken;
  public:
    Meta() : health(false), index(0) {}
    Meta(vector<Instruction> actionBase, size_t index = 0, Token mainToken = Token()) : 
      health(true), index(index) {
      this->actionBase = actionBase;
      this->mainToken = mainToken;
    }

    vector<Instruction> &GetContains() { return actionBase; }
    size_t GetIndex() const { return index; }
    bool IsHealth() const { return health; }
    Token GetMainToken() const { return mainToken; }
  };

  using MachCtlBlk = struct {
    size_t current;
    stack<size_t> cycleNestStack, cycleTailStack, modeStack;
    stack<bool> conditionStack;
    size_t currentMode;
    int nestHeadCount;
    bool sContinue, sBreak, lastIndex, tailRecursion, tailCall;
    vector<string> defHead;
    size_t defStart;
    ObjectMap recursionMap;
  };

  /* Origin index and string data */
  using StringUnit = pair<size_t, string>;

  class Machine {
    vector<Meta> storage;
    vector<string> parameters;
    bool health, isMain, isFunc;

    void ResetBlock(MachCtlBlk *blk);
    void ResetContainer(string funcId);
    void CaseHead(Message &msg, MachCtlBlk *blk);
    void WhenHead(bool value, MachCtlBlk *blk);
    void ConditionRoot(bool value, MachCtlBlk *blk);
    void ConditionBranch(bool value, MachCtlBlk *blk);
    void ConditionLeaf(MachCtlBlk *blk);
    void HeadSign(bool value, MachCtlBlk *blk);
    void TailSign(MachCtlBlk *blk);
    void Continue(MachCtlBlk *blk);
    void Break(MachCtlBlk *blk);
    void MakeFunction(size_t start, size_t end, vector<string> &defHead);
    static bool IsBlankStr(string target);
    Message MetaProcessing(Meta &meta, string name, MachCtlBlk *blk);
    Message PreProcessing();
    void InitGlobalObject(bool createContainer,string name);
  public:
    Machine() : health(false), isMain(false), isFunc(false) {}

    Machine(const Machine &machine) :
      health(machine.health),
      isMain(machine.isMain),
      isFunc(machine.isFunc) {
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }

    Machine(Machine &&machine) :
      Machine(machine) {
      this->isMain = false;
    }

    Machine(vector<Meta> storage) :
      health(true),
      isMain(false),
      isFunc(false) {
      this->storage = storage;
    }

    void operator=(Machine &machine){
      this->storage = machine.storage;
      this->parameters = machine.parameters;
      this->isMain = false;
    }

    void operator=(Machine &&machine) {
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }

    Machine &SetFunc() { 
      this->isFunc = true; 
      return *this;
    }

    Machine &SetMain() {
      this->isMain = true;
      return *this;
    }

    bool GetHealth() const { return health; }

    Machine &SetParameters(vector<string> parms);
    explicit Machine(const char *target, bool isMain = true);
    Message Run(bool createContainer = true, string name = kStrEmpty);
    Message RunAsFunction(ObjectMap &p);
    void Reset(MachCtlBlk *blk);
  };

  void Activiate();
  void InitPlanners();
#if defined(_WIN32)
  void InitLibraryHandler();
#endif
#if defined(_ENABLE_DEBUGGING_) || not defined(_DISABLE_SDL_)
  void LoadSDLStuff();
#endif
  Message FunctionTunnel(ObjectMap &p);
  Message Calling(Activity activity, string args, vector<Object> objects);
  std::wstring s2ws(const std::string &s);
  std::string ws2s(const std::wstring &s);
  Message CheckEntryAndStart(string id, string typeId, ObjectMap &parm);
  bool IsStringObject(Object &obj);
}




