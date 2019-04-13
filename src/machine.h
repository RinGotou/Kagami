#pragma once
/*
   Main virtual machine implementation of Kagami Project
*/
#include "trace.h"
#include "management.h"

#define CHECK_PRINT_OPT()                          \
  if (p.find(kStrSwitchLine) != p.end()) {         \
    std::cout << std::endl;                        \
  }

#define EXPECT_TYPE(_Map, _Item, _Type)            \
  if (!_Map.CheckTypeId(_Item, _Type))             \
    return Message(kCodeIllegalParam,              \
    "Expect object type - " + _Type + ".",         \
    kStateError)

#define EXPECT(_State, _Mess)                      \
  if (!(_State)) return Message(kCodeIllegalParam, _Mess, kStateError)

#define REQUIRED_ARG_COUNT(_Size)                  \
  if (args.size() != _Size) {                   \
    worker.MakeError("Argument is missing.");   \
    return;                                     \
  }

namespace kagami {
  PlainType FindTypeCode(string type_id);
  int64_t IntProducer(Object &obj);
  double FloatProducer(Object &obj);
  string StringProducer(Object &obj);
  bool BoolProducer(Object &obj);
  std::wstring s2ws(const std::string &s);
  std::string ws2s(const std::wstring &s);
  string ParseRawString(const string &src);
  void InitPlainTypes();

  using TypeKey = pair<string, PlainType>;
  const map<string, PlainType> kTypeStore = {
    TypeKey(kTypeIdInt, kPlainInt),
    TypeKey(kTypeIdFloat, kPlainFloat),
    TypeKey(kTypeIdString, kPlainString),
    TypeKey(kTypeIdBool, kPlainBool)
  };

  const vector<GenericToken> kStringOpStore = {
    kTokenPlus, kTokenNotEqual, kTokenEquals
  };

  using ResultTraitKey = pair<PlainType, PlainType>;
  using TraitUnit = pair<ResultTraitKey, PlainType>;
  const map<ResultTraitKey, PlainType> kResultDynamicTraits = {
    TraitUnit(ResultTraitKey(kPlainInt, kPlainInt), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainString, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainBool), kPlainBool),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainBool), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainInt), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainBool), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainString, kPlainInt), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainFloat), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainBool), kPlainString)
  };

  template <class ResultType, class Tx, class Ty, GenericToken op>
  struct BinaryOpBox {
    ResultType Do(Tx A, Ty B) {
      return Tx();
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenPlus> {
    ResultType Do(Tx A, Ty B) {
      return A + B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenMinus> {
    ResultType Do(Tx A, Ty B) {
      return A - B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenTimes> {
    ResultType Do(Tx A, Ty B) {
      return A * B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenDivide> {
    ResultType Do(Tx A, Ty B) {
      return A / B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenEquals> {
    bool Do(Tx A, Ty B) {
      return A == B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenLessOrEqual> {
    bool Do(Tx A, Ty B) {
      return A <= B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenGreaterOrEqual> {
    bool Do(Tx A, Ty B) {
      return A >= B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenNotEqual> {
    bool Do(Tx A, Ty B) {
      return A != B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenGreater> {
    bool Do(Tx A, Ty B) {
      return A > B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenLess> {
    bool Do(Tx A, Ty B) {
      return A < B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenAnd> {
    bool Do(Tx A, Ty B) {
      return A && B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenOr> {
    bool Do(Tx A, Ty B) {
      return A || B;
    }
  };

  //Dispose divide operation for bool type
  template <>
  struct BinaryOpBox<bool, bool, bool, kTokenDivide> {
    bool Do(bool A, bool B) {
      return true;
    }
  };

#define DISPOSE_STRING_MATH_OP(_OP)                 \
  template <>                                       \
  struct BinaryOpBox<string, string, string, _OP> { \
    string Do(string A, string B) {                 \
      return string();                              \
    }                                               \
  }                                                 \

#define DISPOSE_STRING_LOGIC_OP(_OP)                \
  template <>                                       \
  struct BinaryOpBox<bool, string, string, _OP> {   \
    bool Do(string A, string B) {                   \
      return false;                                 \
    }                                               \
  }                                                 \

  DISPOSE_STRING_MATH_OP(kTokenMinus);
  DISPOSE_STRING_MATH_OP(kTokenTimes);
  DISPOSE_STRING_MATH_OP(kTokenDivide);
  DISPOSE_STRING_LOGIC_OP(kTokenLessOrEqual);
  DISPOSE_STRING_LOGIC_OP(kTokenGreaterOrEqual);
  DISPOSE_STRING_LOGIC_OP(kTokenGreater);
  DISPOSE_STRING_LOGIC_OP(kTokenLess);
  DISPOSE_STRING_LOGIC_OP(kTokenAnd);
  DISPOSE_STRING_LOGIC_OP(kTokenOr);

#undef DISPOSE_STRING_MATH_OP
#undef DISPOSE_STRING_LOGIC_OP

  template <class ResultType, GenericToken op>
  using MathBox = BinaryOpBox<ResultType, ResultType, ResultType, op>;

  template <class Tx, GenericToken op>
  using LogicBox = BinaryOpBox<bool, Tx, Tx, op>;

  class Machine;

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
    bool void_call;
    size_t origin_idx;
    size_t logic_idx;
    size_t idx;
    size_t fn_idx;
    size_t skipping_count;
    Interface *invoking_dest;
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
      void_call(false),
      origin_idx(0),
      logic_idx(0),
      idx(0),
      fn_idx(0),
      skipping_count(0),
      invoking_dest(nullptr),
      last_command(kTokenNull),
      mode(kModeNormal),
      error_string(),
      return_stack(),
      mode_stack(),
      condition_stack(),
      loop_head(),
      loop_tail(),
      fn_string_vec() {}

    void MakeError(string str);
    void SwitchToMode(MachineMode mode);
    void RefreshReturnStack(Object obj = Object());
    void GoLastMode();
    bool NeedSkipping();
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

    Object FetchPlainObject(Argument &arg);
    Object FetchInterfaceObject(string id, string domain);
    string FetchDomain(string id, ArgumentType type);
    Object FetchObject(Argument &arg, bool checking = false);

    bool _FetchInterface(InterfacePointer &interface, string id, string type_id);
    bool FetchInterface(InterfacePointer &interface, CommandPointer &command,
      ObjectMap &obj_map);

    void InitFunctionCatching(ArgumentList &args);
    void FinishFunctionCatching(bool closure = false);

    void Skipping(bool enable_terminators, 
      initializer_list<GenericToken> terminators = {});

    Message Invoke(Object obj, string id, 
      const initializer_list<NamedObject> &&args = {});

    void SetSegmentInfo(ArgumentList &args, bool cmd_info = false);
    void CommandHash(ArgumentList &args);
    void CommandSwap(ArgumentList &args);
    void CommandIfOrWhile(GenericToken token, ArgumentList &args);
    void CommandForEach(ArgumentList &args);
    void ForEachChecking(ArgumentList &args);
    void CommandElse();
    void CommandCase(ArgumentList &args);
    void CommandWhen(ArgumentList &args);
    void CommandContinueOrBreak(GenericToken token);
    void CommandConditionEnd();
    void CommandLoopEnd();
    void CommandForEachEnd();

    void CommandBind(ArgumentList &args);
    void CommandTypeId(ArgumentList &args);
    void CommandMethods(ArgumentList &args);
    void CommandExist(ArgumentList &args);
    void CommandNullObj(ArgumentList &args);
    void CommandDestroy(ArgumentList &args);
    void CommandConvert(ArgumentList &args);
    void CommandRefCount(ArgumentList &args);
    void CommandTime();
    void CommandVersion();
    void CommandPatch();

    template <GenericToken op_code>
    void BinaryMathOperatorImpl(ArgumentList &args);

    template <GenericToken op_code>
    void BinaryLogicOperatorImpl(ArgumentList &args);

    void OperatorLogicNot(ArgumentList &args);

    void ExpList(ArgumentList &args);
    void InitArray(ArgumentList &args);
    void DomainAssert(ArgumentList &args);

    void CommandReturn(ArgumentList &args);
    void MachineCommands(GenericToken token, ArgumentList &args, Request &request);

    void GenerateArgs(Interface &interface, ArgumentList &args, ObjectMap &obj_map);
    void Generate_Normal(Interface &interface, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoSize(Interface &interface, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoFill(Interface &interface, ArgumentList &args, ObjectMap &obj_map);
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

  bool IsStringFamily(Object &obj);
}
