#pragma once

#include <fstream>
#include "trace.h"
#include "management.h"

#define SET_MAP(MAP) auto &obj_map = MAP

#define CONVERT_OBJECT(ID,TYPE) obj_map[ID].Cast<TYPE>()

#define EXPECT_TYPE(MAP,ITEM,TYPE)                 \
  if (!MAP.CheckTypeId(ITEM,TYPE))                 \
    return Message(kCodeIllegalParam,              \
    "Expected object type - " + TYPE + ".",        \
    kStateError)

#define CHECK_OBJECT_TYPE(ID,TYPEID)               \
  EXPECT_TYPE(obj_map, ID, TYPEID)

#define EXPECT(STATE,MESS)                         \
  if (!(STATE)) return Message(kCodeIllegalParam,MESS,kStateError)


namespace kagami {
  template <class T>
  shared_ptr<void> SimpleSharedPtrCopy(shared_ptr<void> target) {
    T temp(*static_pointer_cast<T>(target));
    return make_shared<T>(temp);
  }

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
    void ConditionIf(bool value);
    void ConditionElif(bool value);
    bool ConditionElse();
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
    stack<Object> returning_base;

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

    void MakeCode(StateCode code) {
      deliver = true;
      msg = Message(code, "");
    }

    Object MakeObject(Argument &arg, bool checking = false);
    void Assembling_AutoSize(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Assembling_AutoFill(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Assembling(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Reset();

    bool Bind(ArgumentList args);
    bool ExpList(ArgumentList args);
    bool InitArray(ArgumentList args);
    bool ReturnOperator(ArgumentList args);
    bool GetTypeId(ArgumentList args);
    bool GetMethods(ArgumentList args);
    bool Exist(ArgumentList args);
    bool Fn(ArgumentList args);
    bool Case(ArgumentList args);
    bool When(ArgumentList args);
    bool DomainAssert(ArgumentList args, bool returning);
    bool ConditionAndLoop(ArgumentList args, StateCode code);
  };

  class IRMaker {
  public:
    bool health;
    vector<IR> output;

    IRMaker() {}
    IRMaker(const char *path);
  };

  class Module {
  private:
    vector<IR> storage_;
    bool health_, is_main_;

  private:
    void ResetContainer(string funcId);
    void MakeFunction(size_t start, size_t end, vector<string> &defHead);
    static bool IsBlankStr(string target);
    Message IRProcessing(IR &IL_set, string name, MachCtlBlk *blk);
    Message PreProcessing();
    void InitGlobalObject(bool create_container,string name);
    bool PredefinedMessage(size_t mode, Token token, Message &msg);
    void TailRecursionActions(MachCtlBlk *blk, string &name);
    void CallMachineFunction(StateCode code, string detail, MachCtlBlk *blk);
    bool GenericRequests(IRWorker *worker, Request &Request, ArgumentList &args);
    bool CheckGenericRequests(GenericToken token);

  public:
    Module() : 
      health_(false), 
      is_main_(false) {}

    Module(const Module &module) :
      health_(module.health_),
      is_main_(module.is_main_) {
      storage_ = module.storage_;
    }

    Module(Module &&module) :
      Module(module) {

      is_main_ = false;
    }

    Module(vector<IR> storage) :
      health_(true),
      is_main_(false) {

      storage_ = storage;
    }

    Module(IRMaker &maker, bool is_main) :
      health_(true),
      is_main_(is_main) {

      Message msg;

      if (maker.health) {
        storage_ = maker.output;
        msg = PreProcessing();
        if (msg.GetLevel() == kStateError) {
          health_ = false;
          trace::AddEvent(msg);
        }
      }
      else {
        health_ = false;
        trace::AddEvent(Message(kCodeBadStream, "Invalid script.", kStateError));
      }
    }

    void operator=(Module &module) {
      storage_ = module.storage_;
      is_main_ = false;
    }

    void operator=(Module &&module) {
      return this->operator=(module);
    }

    Module &SetMain() {
      is_main_ = true;
      return *this;
    }

    bool Good() const { 
      return health_; 
    }

    Message Run(bool create_container = true, string name = "");
    Message RunAsFunction(ObjectMap &p);
    void Reset(MachCtlBlk *blk);
  };

  void Activiate();
  void InitBaseTypes();
#if not defined(_DISABLE_SDL_)
  void LoadSDLStuff();
#endif

#if defined(_WIN32)
  void LoadSocketStuff();
#else
  //TODO:Reserved for unix socket wrapper
  //TODO: delete macros after finish it
#endif

  std::wstring s2ws(const std::string &s);
  std::string ws2s(const std::wstring &s);
  bool IsStringObject(Object &obj);
  Object GetFunctionObject(string id, string domain);
  shared_ptr<void> FakeCopy(shared_ptr<void> target);
  string ParseRawString(const string &src);
  bool IsStringFamily(Object &obj);
  void CopyObject(Object &dest, Object &src);
}
