#pragma once

#include <fstream>
#include "trace.h"
#include "management.h"

#define OBJECT_ASSERT(MAP,ITEM,TYPE)               \
  if (!MAP.CheckTypeId(ITEM,TYPE))                 \
    return Message(kCodeIllegalParm,               \
    "Expected object type - " + TYPE + ".",        \
    kStateError);

#define CONDITION_ASSERT(STATE,MESS)               \
  if(!(STATE)) return Message(kCodeIllegalParm,MESS,kStateError);

#define CALL_ASSERT(STATE,MESS)                    \
  if(!(STATE)) return Message(kCodeIllegalCall,MESS,kStateError);

#define ASSERT_RETURN(STATE,VALUE)                 \
  if(!(STATE)) return Message(VALUE);

#define CUSTOM_ASSERT(STATE,CODE,MESS)             \
  if(!(STATE)) return Message(CODE,MESS,kStateError);


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

  class IR {
  private:
    vector<Command> container_;
    size_t index_;
    Token main_token_;
  public:
    IR() : 
      index_(0) {}

    IR(vector<Command> commands, 
      size_t index = 0, 
      Token main_token = Token()) : 
      index_(index) {

      container_ = commands;
      this->main_token_ = main_token;
    }

    vector<Command> &GetContains() { 
      return container_; 
    }

    size_t GetIndex() const { 
      return index_; 
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
    MachineMode mode;
    int nest_head_count;
    string error_string;
    stack<size_t> cycle_nest, cycle_tail;
    stack<MachineMode> mode_stack;
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

    void Case();
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

  class IRMaker {
  public:
    bool health;
    vector<IR> output;

    IRMaker() {}
    IRMaker(const char *path);
  };

  class Module {
    vector<IR> storage_;
    vector<string> parameters_;
    bool health_, is_main_, is_func_;

    void ResetContainer(string funcId);
    void MakeFunction(size_t start, size_t end, vector<string> &defHead);
    static bool IsBlankStr(string target);
    Message IRProcessing(IR &IL_set, string name, MachCtlBlk *blk);
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
    bool ConditionAndLoop(IRWorker *worker, deque<Argument> args, StateCode code);

    //Command Management
    bool GenericRequests(IRWorker *worker, Request &Request, deque<Argument> &args);
    bool CheckGenericRequests(GenericTokenEnum token);
  public:
    Module() : 
      health_(false), 
      is_main_(false), 
      is_func_(false) {}

    Module(const Module &module) :
      health_(module.health_),
      is_main_(module.is_main_),
      is_func_(module.is_func_) {
      storage_ = module.storage_;
      parameters_ = module.parameters_;
    }

    Module(Module &&module) :
      Module(module) {

      is_main_ = false;
    }

    Module(vector<IR> storage) :
      health_(true),
      is_main_(false),
      is_func_(false) {

      storage_ = storage;
    }

    Module(IRMaker &maker, bool is_main) :
      health_(true),
      is_main_(is_main),
      is_func_(false) {

      Message msg;

      if (maker.health) {
        storage_ = maker.output;
        msg = PreProcessing();
        if (msg.GetLevel() == kStateError) {
          health_ = false;
          trace::Log(msg);
        }
      }
      else {
        health_ = false;
        trace::Log(Message(kCodeBadStream, "Invalid script path.", kStateError));
      }
    }

    void operator=(Module &module) {
      storage_ = module.storage_;
      parameters_ = module.parameters_;
      is_main_ = false;
    }

    void operator=(Module &&module) {
      storage_ = module.storage_;
      parameters_ = module.parameters_;
    }

    Module &SetFunc() { 
      is_func_ = true; 
      return *this;
    }

    Module &SetMain() {
      is_main_ = true;
      return *this;
    }

    bool GetHealth() const { 
      return health_; 
    }

    Module &SetParameters(vector<string> parms) {
      parameters_ = parms;
      return *this;
    }
    Message Run(bool create_container = true, string name = "");
    Message RunAsFunction(ObjectMap &p);
    void Reset(MachCtlBlk *blk);
  };

  void Activiate();
  void InitPlanners();
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
