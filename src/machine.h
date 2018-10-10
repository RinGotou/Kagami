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
    bool sContinue, sBreak;
    vector<string> defHead;
    size_t defStart;
  };

  /* Origin index and string data */
  using StringUnit = pair<size_t, string>;

  class Machine {
    vector<Meta> storage;
    vector<string> parameters;
    bool health, isMain;

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
    Message MetaProcessing(Meta &meta);
    Message PreProcessing();
    void InitGlobalObject(bool createContainer);
  public:
    Machine() : health(false), isMain(false) {}
    Machine(const Machine &machine) : health(machine.health), isMain(machine.isMain) {
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }
    Machine(Machine &&machine) : Machine(machine) {}
    Machine(vector<Meta> storage) : health(true) {
      this->storage = storage;
    }
    void operator=(Machine &machine){
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }
    void operator=(Machine &&machine) {
      this->storage = machine.storage;
      this->parameters = machine.parameters;
    }

    Machine &SetParameters(vector<string> parms);
    explicit Machine(const char *target, bool isMain = true);
    Message Run(bool createContainer = true);
    //Message Run2(bool createContainer = true);
    Message RunAsFunction(ObjectMap &p);
    void Reset(MachCtlBlk *blk);

    bool GetHealth() const { return health; }
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




