#pragma once
/*
   Kisaragi(February) - Next Kagami IR Interpreter Framework
*/
#include "trace.h"
#include "management.h"

#define CHECK_PRINT_OPT()                          \
  if (p.find(kStrSwitchLine) != p.end()) {         \
    std::cout << std::endl;                        \
  }

#define SET_MAP(MAP) auto &obj_map = MAP

#define CONVERT_OBJECT(ID,TYPE) obj_map[ID].Cast<TYPE>()

#define EXPECT_TYPE(MAP,ITEM,TYPE)                 \
  if (!MAP.CheckTypeId(ITEM,TYPE))                 \
    return Message(kCodeIllegalParam,              \
    "Expect object type - " + TYPE + ".",          \
    kStateError)

#define CHECK_OBJECT_TYPE(ID,TYPEID)               \
  EXPECT_TYPE(obj_map, ID, TYPEID)

#define EXPECT(STATE,MESS)                         \
  if (!(STATE)) return Message(kCodeIllegalParam,MESS,kStateError)

#define INVALID_CALL_MSG(MSG) Message(kCodeIllegalCall, MSG, kStateError)

#define INVALID_PARAM_MSG(MSG) Message(kCodeIllegalParam, MSG, kStateError)

#define BAD_EXP_MSG(MSG) Message(kCodeBadExpression, MSG, kStateError)

namespace kagami {
  const string kIteratorBehavior = "get|step_forward|step_back|__compare";
  const string kContainerBehavior = "head|tail";
  using CombinedCodeline = pair<size_t, string>;
  using CommandPointer = Command * ;

  template <class T>
  shared_ptr<void> SimpleSharedPtrCopy(shared_ptr<void> target) {
    T temp(*static_pointer_cast<T>(target));
    return make_shared<T>(temp);
  }

  template <class T>
  shared_ptr<void> FakeCopy(shared_ptr<void> target) {
    return target;
  }

  class MachineWorker {
  public:
    bool error;
    bool activated_continue;
    bool activated_break;
    size_t origin_idx;
    size_t logic_idx;
    size_t idx;
    size_t fn_idx;
    size_t skipping_count;
    GenericToken last_command;
    MachineMode mode;
    string error_string;
    stack<Object> return_stack;
    stack<MachineMode> mode_stack;
    stack<bool> condition_stack;
    stack<size_t> loop_head;
    stack<size_t> loop_tail;
    vector<string> fn_string_vec;

    MachineWorker() :
      error(false),
      activated_continue(false),
      activated_break(false),
      origin_idx(0),
      logic_idx(0),
      idx(0),
      fn_idx(0),
      skipping_count(0),
      last_command(kTokenNull),
      mode(kModeNormal),
      error_string(),
      return_stack(),
      mode_stack(),
      condition_stack(),
      loop_head(),
      loop_tail(),
      fn_string_vec() {}

    void MakeError(string str) {
      error = true;
      error_string = str;
    }

    void SwitchToMode(MachineMode mode) {
      mode_stack.push(this->mode);
      this->mode = mode;
    }

    void GoLastMode() {
      if (!mode_stack.empty()) {
        this->mode = mode_stack.top();
        mode_stack.pop();
      }
      else {
        MakeError("Mode switching error.Kisaragi Kernel Panic.");
      }
    }

    bool NeedSkipping() {
      switch (mode) {
      case kModeNextCondition:
      case kModeCycleJump:
      case kModeDef:
      case kModeCaseJump:
      case kModeForEachJump:
      case kModeClosureCatching:
        return true;
        break;
      default:
        break;
      }
      return false;
    }
  };

  class IRLoader {
  public:
    bool health;
    KIR output;

    IRLoader(const char *src);

  private:
    string IndentationAndCommentProc(string target);
  };

  //Kisaragi Machine Class
  class Machine {
  private:
    void RecoverLastState();

    Object FetchPlainObject(string value);
    Object FetchInterfaceObject(string id, string domain);
    Object FetchObject(Argument &arg, bool checking = false);

    bool _FetchInterface(Interface &interface, string id, string type_id);
    bool FetchInterface(Interface &interface, CommandPointer &command,
      ObjectMap &obj_map);

    void InitFunctionCatching(ArgumentList args);
    void FinishFunctionCatching(bool closure = false);

    void Skipping(bool enable_terminators, 
      initializer_list<GenericToken> terminators = {});

    Message Invoke(Object obj, string id, 
      const initializer_list<NamedObject> &&args = {});

    void SetSegmentInfo(ArgumentList args);
    void CommandSwap(ArgumentList args);
    void CommandIfOrWhile(GenericToken token, ArgumentList args);
    void CommandForEach(ArgumentList args);
    void ForEachChecking(ArgumentList args);
    void CommandElse();
    void CommandCase(ArgumentList args);
    void CommandWhen(ArgumentList args);
    void CommandContinueOrBreak(GenericToken token);
    void CommandConditionEnd();
    void CommandLoopEnd();
    void CommandForEachEnd();

    void CommandBind(ArgumentList args);
    void CommandTypeId(ArgumentList args);
    void CommandMethods(ArgumentList args);
    void CommandExist(ArgumentList args);
    void CommandNullObj(ArgumentList args);
    void CommandDestroy(ArgumentList args);
    void CommandConvert(ArgumentList args);
    void CommandRefCount(ArgumentList args);
    void CommandTime();
    void CommandVersion();
    void CommandPatch();
    void ExpList(ArgumentList args);
    void InitArray(ArgumentList args);
    void DomainAssert(ArgumentList args, bool returning, bool no_feeding);

    void CommandReturn(ArgumentList args);
    void MachineCommands(GenericToken token, ArgumentList args, Request request);

    void GenerateArgs(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Generate_Normal(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Generate_AutoSize(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Generate_AutoFill(Interface &interface, ArgumentList args, ObjectMap &obj_map);
  private:
    deque<KIRPointer> ir_stack_;
    stack<MachineWorker> worker_stack_;
    ObjectStack obj_stack_;

  public:
    Machine() :
      ir_stack_(),
      worker_stack_(),
      obj_stack_() {}

    Machine(const Machine &rhs) :
      ir_stack_(rhs.ir_stack_),
      worker_stack_(rhs.worker_stack_),
      obj_stack_(rhs.obj_stack_) {}

    Machine(const Machine &&rhs) :
      Machine(rhs) {}

    Machine(KIR &ir) :
      ir_stack_(),
      worker_stack_(),
      obj_stack_() {
      ir_stack_.push_back(&ir);
    }

    void SetPreviousStack(ObjectStack &prev) {
      obj_stack_.SetPreviousStack(prev);
    }

    void Run();
  };

  void InitConsoleComponents();
  void InitPlainTypeComponents();
  void InitBaseTypes();
  void InitContainerComponents();

#if not defined(_DISABLE_SDL_)
  void InitSoundComponents();
#endif

#if defined(_WIN32)
  void LoadSocketStuff();
#else
  //TODO:Reserved for unix socket wrapper
  //TODO: delete macros after finish it
#endif

  std::wstring s2ws(const std::string &s);
  std::string ws2s(const std::wstring &s);
  string ParseRawString(const string &src);
  bool IsStringFamily(Object &obj);
}
