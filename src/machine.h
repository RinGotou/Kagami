#pragma once

#include <fstream>
#include "trace.h"

#define OBJECT_ASSERT(MAP,ITEM,TYPE)               \
  if (!MAP.CheckTypeId(ITEM,TYPE))                 \
    return Message(kStrFatalError,kCodeIllegalParm,\
    "Expected object type - " + TYPE + ".");

#define CONDITION_ASSERT(STATE,MESS)               \
  if(!(STATE)) return Message(kStrFatalError,kCodeIllegalParm,MESS);

#define CALL_ASSERT(STATE,MESS)                    \
  if(!(STATE)) return Message(kStrFatalError,kCodeIllegalCall,MESS);

#define ASSERT_RETURN(STATE,VALUE)                 \
  if(!(STATE)) return Message(VALUE);

#define CUSTOM_ASSERT(STATE,CODE,MESS)             \
  if(!(STATE)) return Message(kStrFatalError,CODE,MESS);


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

  class KILSet {
    bool health_;
    vector<KIL> action_base_;
    size_t index_;
    Token main_token_;
  public:
    KILSet() : 
      health_(false), 
      index_(0) {}

    KILSet(vector<KIL> actionBase, 
      size_t index = 0, 
      Token mainToken = Token()) : 
      health_(true), 
      index_(index) {

      this->action_base_ = actionBase;
      this->main_token_ = mainToken;
    }

    vector<KIL> &GetContains() { 
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

  class MachCtlBlk {
  public:
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

    MachCtlBlk():
      s_continue(false),
      s_break(false),
      last_index(false),
      tail_recursion(false),
      tail_call(false),
      runtime_error(false),
      current(0),
      def_start(0),
      mode(kModeNormal),
      nest_head_count(0),
      error_string() {}

    void Case(Message &msg);
    void When(bool value);
    void ConditionRoot(bool value);
    void ConditionBranch(bool value);
    bool ConditionLeaf();
    void LoopHead(bool value);
    void End();
    void Continue();
    void Break();
    void Clear();
  };

  class IRWorker {
  public:
    string error_string;
    bool error_returning,
      error_obj_checking,
      error_assembling,
      is_assert,
      is_assert_r,
      deliver,
      tail_recursion;
    Message msg;
    deque<Object> returning_base;

    IRWorker() :
      error_string(),
      error_returning(false),
      error_obj_checking(false),
      error_assembling(false),
      is_assert(false),
      is_assert_r(false),
      deliver(false),
      tail_recursion(false),
      msg() {}

    Object MakeObject(Argument &arg, bool checking = false);
    void AssemblingForAutoSized(Entry &ent, deque<Argument> parms, ObjectMap &obj_map);
    void AssemblingForAutoFilling(Entry &ent, deque<Argument> parms, ObjectMap &obj_map);
    void AssemblingForNormal(Entry &ent, deque<Argument> parms, ObjectMap &obj_map);
    void Reset();
  };

  class Machine {
    //Machine *parent_;
    vector<KILSet> storage_;
    vector<string> parameters_;
    bool health_, is_main_, is_func_;

    void ResetContainer(string funcId);
    void MakeFunction(size_t start, size_t end, vector<string> &defHead);
    static bool IsBlankStr(string target);
    Message IRProcessing(KILSet &IL_set, string name, MachCtlBlk *blk);
    Message PreProcessing();
    void InitGlobalObject(bool create_container,string name);
    bool PredefinedMessage(Message &result, size_t mode, Token token);
    void TailRecursionActions(MachCtlBlk *blk, string &name);

    //Command Functions
    bool BindAndSet(IRWorker *worker, deque<Argument> args); //Object Management (Old)
    void Nop(IRWorker *worker, deque<Argument> args);        //Bracket    
    void ArrayMaker(IRWorker *worker, deque<Argument> args); //Braces
    void ReturnOperator(IRWorker *worker, deque<Argument> args); //Return
    bool GetTypeId(IRWorker *worker, deque<Argument> args);      //TypeId
    bool GetMethods(IRWorker *worker, deque<Argument> args);     //Dir
    bool Exist(IRWorker *worker, deque<Argument> args);          //Exist
    bool Define(IRWorker *worker, deque<Argument> args);         //Def
    bool Case(IRWorker *worker, deque<Argument> args);
    bool When(IRWorker *worker, deque<Argument> args);
    bool DomainAssert(IRWorker *worker, deque<Argument> args, bool returning);
    void Quit(IRWorker *worker);
    void End(IRWorker *worker);
    void Continue(IRWorker *worker);
    void Break(IRWorker *worker);
    void Else(IRWorker *worker);
    bool ConditionAndLoop(IRWorker *worker, deque<Argument> args, int code);

    //Command Management
    bool GenericRequests(IRWorker *worker, Request &Request, deque<Argument> &args);
    bool CheckGenericRequests(GenericTokenEnum token);
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

    Machine(vector<KILSet> storage) :
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
#if not defined(_DISABLE_SDL_)
  void LoadSDLStuff();
#endif
  Message FunctionTunnel(ObjectMap &p);
  std::wstring s2ws(const std::string &s);
  std::string ws2s(const std::wstring &s);
  Message CheckEntryAndStart(string id, string type_id, ObjectMap &parm);
  bool IsStringObject(Object &obj);
  Object GetFunctionObject(string id, string domain);
  shared_ptr<void> FakeCopy(shared_ptr<void> target);
  shared_ptr<void> NullCopy(shared_ptr<void> target);
  string RealString(const string &src);
  bool IsStringFamily(Object &obj);
  void CopyObject(Object &dest, Object &src);
}
