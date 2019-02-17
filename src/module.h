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

#define INVALID_CALL_MSG(MSG) Message(kCodeIllegalCall, MSG, kStateError)

#define INVALID_PARAM_MSG(MSG) Message(kCodeIllegalParam, MSG, kStateError)

#define BAD_EXP_MSG(MSG) Message(kCodeBadExpression, MSG, kStateError)

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
    size_t fn_idx;
    size_t current;
    size_t def_start;
    MachineMode mode;
    string error_string;
    stack<size_t> cycle_nest, cycle_tail;
    stack<MachineMode> mode_stack;
    stack<bool> condition_stack;
    vector<string> func_string_vec;
    ObjectMap recursion_map;

    MachCtlBlk():
      s_continue(false),
      s_break(false),
      last_index(false),
      tail_recursion(false),
      tail_call(false),
      runtime_error(false),
      fn_idx(0),
      current(0),
      def_start(0),
      mode(kModeNormal),
      error_string() {}

    void SetError(string str) {
      runtime_error = true;
      error_string = str;
    }

    void Case();
    void When(bool value);
    void ConditionIf(bool value);
    void ConditionElif(bool value);
    void ConditionElse();
    void LoopHead(bool value);
    void End(vector<IR> &storage);
    void Continue();
    void Break();
    void Clear();
    void CreateClosureProc(string func_string);
    void CatchClosure(vector<IR> &storage);
  };

  class IRWorker {
  public:
    string error_string;
    bool error,
      deliver,
      tail_recursion;
    Message msg;
    stack<Object> returning_base;

    IRWorker() :
      error_string(),
      error(false),
      deliver(false),
      tail_recursion(false),
      msg() {}

    void MakeCode(StateCode code) {
      deliver = true;
      msg = Message(code, "");
    }

    void MakeMsg(Message msg) {
      deliver = true;
      this->msg = msg;
    }

    Message GetMsg() {
      deliver = false;
      return msg;
    }

    void MakeError(string str) {
      error = true;
      error_string = str;
    }

    Object MakeObject(Argument &arg, bool checking = false);
    void Assembling_AutoSize(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Assembling_AutoFill(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Assembling(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void GenerateArgs(StateCode code, Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Reset();

    void Bind(ArgumentList args);
    void ExpList(ArgumentList args);
    void InitArray(ArgumentList args);
    void ReturnOperator(ArgumentList args);
    void GetTypeId(ArgumentList args);
    void GetMethods(ArgumentList args);
    void Exist(ArgumentList args);
    void Fn(ArgumentList args);
    void Case(ArgumentList args);
    void When(ArgumentList args);
    void DomainAssert(ArgumentList args, bool returning, bool no_feeding);
    void ConditionAndLoop(ArgumentList args, StateCode code);
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
    bool is_main_;

  private:
    void ResetContainer(string funcId);
    void MakeFunction(size_t start, size_t end, vector<string> &defHead);
    Message IRProcessing(IR &IL_set, string name, MachCtlBlk *blk);
    Message PreProcessing();
    void TailRecursionActions(MachCtlBlk *blk, string &name);
    void CallMachineFunction(StateCode code, string detail, MachCtlBlk *blk);
    void GenericRequests(IRWorker *worker, Request &Request, ArgumentList &args);

    bool SkippingWithCondition(MachCtlBlk *blk,
      std::initializer_list<GenericToken> terminators);
    bool Skipping(MachCtlBlk *blk);
    bool SkippingStrategy(MachCtlBlk *blk);

    bool NeedSkipping(MachineMode mode) {
      switch (mode) {
      case kModeNextCondition:
      case kModeCycleJump:
      case kModeDef:
      case kModeCaseJump:
      case kModeClosureCatching:
        return true;
        break;
      }
      return false;
    }

  public:
    Module() : 
      is_main_(false) {}

    Module(const Module &module) :
      is_main_(module.is_main_) {
      storage_ = module.storage_;
    }

    Module(Module &&module) :
      Module(module) {

      is_main_ = false;
    }

    Module(vector<IR> storage) :
      is_main_(false),
      storage_(std::move(storage)) {}

    Module(IRMaker &maker, bool is_main);

    void operator=(Module &module) {
      storage_ = module.storage_;
      is_main_ = false;
    }

    void operator=(Module &&module) {
      return this->operator=(module);
    }

    Message Run(bool create_container = true, string name = "");
    Message RunAsFunction(ObjectMap &p);
    void Reset(MachCtlBlk *blk);
  };

  void Activiate();
  void InitBaseTypes();
  void InitContainerComponents();

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
