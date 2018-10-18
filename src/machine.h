#pragma once

#include <fstream>
#include "trace.h"

namespace kagami {
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
    return Object().Manage(str, util::GetTokenType(str)).set_ro(false);
  }

  class Meta {
    bool health_;
    vector<Instruction> action_base_;
    size_t index_;
    Token main_token_;
  public:
    Meta() : 
      health_(false), 
      index_(0) {}

    Meta(vector<Instruction> actionBase, 
      size_t index = 0, 
      Token mainToken = Token()) : 
      health_(true), 
      index_(index) {

      this->action_base_ = actionBase;
      this->main_token_ = mainToken;
    }

    vector<Instruction> &GetContains() { 
      return action_base_; 
    }

    size_t GetIndex() const { 
      return index_; 
    }

    bool IsHealth() const { 
      return health_; 
    }

    Token GetMainToken() const { 
      return main_token_; 
    }
  };

  /* Origin index and string data */
  using StringUnit = pair<size_t, string>;

  class Machine {
    using MachCtlBlk = struct {
      bool s_continue, 
        s_break, 
        last_index, 
        tail_recursion, 
        tail_call, 
        runtime_error;
      size_t current;
      size_t def_start;
      size_t mode;
      int nest_head_count;
      string error_string;
      stack<size_t> cycle_nest, cycle_tail, mode_stack;
      stack<bool> condition_stack;
      vector<string> def_head;
      ObjectMap recursion_map;
    };

    using MetaWorkBlock = struct {
      string error_string;
      deque<Object> returning_base;
      bool error_returning,
        error_obj_checking,
        error_assembling,
        is_assert,
        is_assert_r,
        tail_recursion;
    };

    vector<Meta> storage_;
    vector<string> parameters_;
    bool health_, is_main_, is_func_;

    //Machine Controlling Block
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
    void InitGlobalObject(bool create_container,string name);
    bool PredefinedMessage(Message &result, size_t mode, Token token);
    void TailRecursionActions(MachCtlBlk *blk, string &name);

    //Meta Work Block
    Object MakeObject(Argument &arg, MetaWorkBlock *meta_blk, bool checking = false);
    void ResetMetaWorkBlock(MetaWorkBlock *meta_blk);
    void AssemblingForAutosized(Instruction &inst, ObjectMap &obj_map, MetaWorkBlock *meta_blk);
    void AssemblingForAutoFilling(Instruction &inst, ObjectMap &obj_map, MetaWorkBlock *meta_blk);
    void AssemblingForNormal(Instruction &inst, ObjectMap &obj_map, MetaWorkBlock *meta_blk);
  public:
    Machine() : 
      health_(false), 
      is_main_(false), 
      is_func_(false) {}

    Machine(const Machine &machine) :
      health_(machine.health_),
      is_main_(machine.is_main_),
      is_func_(machine.is_func_) {
      storage_ = machine.storage_;
      parameters_ = machine.parameters_;
    }

    Machine(Machine &&machine) :
      Machine(machine) {
      is_main_ = false;
    }

    Machine(vector<Meta> storage) :
      health_(true),
      is_main_(false),
      is_func_(false) {
      storage_ = storage;
    }

    void operator=(Machine &machine){
      storage_ = machine.storage_;
      parameters_ = machine.parameters_;
      is_main_ = false;
    }

    void operator=(Machine &&machine) {
      storage_ = machine.storage_;
      parameters_ = machine.parameters_;
    }

    Machine &SetFunc() { 
      is_func_ = true; 
      return *this;
    }

    Machine &SetMain() {
      is_main_ = true;
      return *this;
    }

    bool GetHealth() const { 
      return health_; 
    }

    Machine &SetParameters(vector<string> parms) {
      parameters_ = parms;
      return *this;
    }

    explicit Machine(const char *target, bool is_main = true);
    Message Run(bool create_container = true, string name = kStrEmpty);
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
  Message CheckEntryAndStart(string id, string type_id, ObjectMap &parm);
  bool IsStringObject(Object &obj);
  Object GetFunctionObject(string id, string domain);
}
