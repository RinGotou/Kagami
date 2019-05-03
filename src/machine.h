#pragma once
/*
   Main virtual machine implementation of Kagami Project
   Machine version : kisaragi
*/
#include "frontend.h"
#include "management.h"

#define CHECK_PRINT_OPT()                          \
  if (p.find(kStrSwitchLine) != p.end()) {         \
    putc('\n', VM_STDOUT);                         \
  }

#define EXPECT_TYPE(_Map, _Item, _Type)            \
  if (!_Map.CheckTypeId(_Item, _Type))             \
    return Message(kCodeIllegalParam,              \
    "Expect object type - " + _Type + ".",         \
    kStateError)

#define EXPECT(_State, _Mess)                      \
  if (!(_State)) return Message(kCodeIllegalParam, _Mess, kStateError)

#define REQUIRED_ARG_COUNT(_Size)                  \
  if (args.size() != _Size) {                      \
    frame.MakeError("Argument is missing.");       \
    return;                                        \
  }

namespace kagami {
  using management::type::PlainComparator;

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

  const vector<Keyword> kStringOpStore = {
    kKeywordPlus, kKeywordNotEqual, kKeywordEquals
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

  template <class ResultType, class Tx, class Ty, Keyword op>
  struct BinaryOpBox {
    ResultType Do(Tx A, Ty B) {
      return Tx();
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordPlus> {
    ResultType Do(Tx A, Ty B) {
      return A + B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordMinus> {
    ResultType Do(Tx A, Ty B) {
      return A - B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordTimes> {
    ResultType Do(Tx A, Ty B) {
      return A * B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordDivide> {
    ResultType Do(Tx A, Ty B) {
      return A / B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordEquals> {
    bool Do(Tx A, Ty B) {
      return A == B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordLessOrEqual> {
    bool Do(Tx A, Ty B) {
      return A <= B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordGreaterOrEqual> {
    bool Do(Tx A, Ty B) {
      return A >= B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordNotEqual> {
    bool Do(Tx A, Ty B) {
      return A != B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordGreater> {
    bool Do(Tx A, Ty B) {
      return A > B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordLess> {
    bool Do(Tx A, Ty B) {
      return A < B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordAnd> {
    bool Do(Tx A, Ty B) {
      return A && B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordOr> {
    bool Do(Tx A, Ty B) {
      return A || B;
    }
  };

  //Dispose divide operation for bool type
  template <>
  struct BinaryOpBox<bool, bool, bool, kKeywordDivide> {
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

  DISPOSE_STRING_MATH_OP(kKeywordMinus);
  DISPOSE_STRING_MATH_OP(kKeywordTimes);
  DISPOSE_STRING_MATH_OP(kKeywordDivide);
  DISPOSE_STRING_LOGIC_OP(kKeywordLessOrEqual);
  DISPOSE_STRING_LOGIC_OP(kKeywordGreaterOrEqual);
  DISPOSE_STRING_LOGIC_OP(kKeywordGreater);
  DISPOSE_STRING_LOGIC_OP(kKeywordLess);
  DISPOSE_STRING_LOGIC_OP(kKeywordAnd);
  DISPOSE_STRING_LOGIC_OP(kKeywordOr);

#undef DISPOSE_STRING_MATH_OP
#undef DISPOSE_STRING_LOGIC_OP

  template <class ResultType, Keyword op>
  using MathBox = BinaryOpBox<ResultType, ResultType, ResultType, op>;

  template <class Tx, Keyword op>
  using LogicBox = BinaryOpBox<bool, Tx, Tx, op>;

  const string kIteratorBehavior = "obj|step_forward|__compare";
  const string kContainerBehavior = "head|tail";

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

  class RuntimeFrame {
  public:
    bool error;
    bool activated_continue;
    bool activated_break;
    bool void_call;
    bool disable_step;
    bool final_cycle;
    bool jump_from_end;
    size_t jump_offset;
    size_t idx;
    string error_string;
    stack<bool> condition_stack; //preserved
    stack<bool> scope_stack;
    stack<size_t> jump_stack;
    stack<size_t> branch_jump_stack;
    stack<Object> return_stack;

    RuntimeFrame() :
      error(false),
      activated_continue(false),
      activated_break(false),
      void_call(false),
      disable_step(false),
      final_cycle(false),
      jump_from_end(false),
      jump_offset(0),
      idx(0),
      error_string(),
      condition_stack(),
      jump_stack(),
      branch_jump_stack(),
      return_stack() {}

    void Steping();
    void Goto(size_t taget_idx);
    void AddJumpRecord(size_t target_idx);
    void MakeError(string str);
    void RefreshReturnStack(Object obj = Object());
  };

  //Kisaragi Machine Class
  class Machine {
  private:
    void RecoverLastState();

    Object FetchPlainObject(Argument &arg);
    Object FetchInterfaceObject(string id);
    Object FetchObject(Argument &arg, bool checking = false);

    bool _FetchInterface(InterfacePointer &interface, string id, string type_id);
    bool FetchInterface(InterfacePointer &interface, CommandPointer &command,
      ObjectMap &obj_map);

    void ClosureCatching(ArgumentList &args, size_t nest_end, bool closure);

    Message Invoke(Object obj, string id, 
      const initializer_list<NamedObject> &&args = {});

    void CommandIfOrWhile(Keyword token, ArgumentList &args, size_t nest_end);
    void CommandForEach(ArgumentList &args, size_t nest_end);
    void ForEachChecking(ArgumentList &args, size_t nest_end);
    void CommandCase(ArgumentList &args, size_t nest_end);
    void CommandElse();
    void CommandWhen(ArgumentList &args);
    void CommandContinueOrBreak(Keyword token, size_t escape_depth);
    void CommandConditionEnd();
    void CommandLoopEnd(size_t nest);
    void CommandForEachEnd(size_t nest);

    void CommandHash(ArgumentList &args);
    void CommandSwap(ArgumentList &args);
    void CommandBind(ArgumentList &args, bool local_value);
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

    template <Keyword op_code>
    void BinaryMathOperatorImpl(ArgumentList &args);

    template <Keyword op_code>
    void BinaryLogicOperatorImpl(ArgumentList &args);

    void OperatorLogicNot(ArgumentList &args);

    void ExpList(ArgumentList &args);
    void InitArray(ArgumentList &args);

    void CommandReturn(ArgumentList &args);
    void MachineCommands(Keyword token, ArgumentList &args, Request &request);

    void GenerateArgs(Interface &interface, ArgumentList &args, ObjectMap &obj_map);
    void Generate_Normal(Interface &interface, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoSize(Interface &interface, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoFill(Interface &interface, ArgumentList &args, ObjectMap &obj_map);
  private:
    deque<VMCodePointer> code_stack_;
    stack<RuntimeFrame> frame_stack_;
    ObjectStack obj_stack_;

  public:
    Machine() :
      code_stack_(),
      frame_stack_(),
      obj_stack_() {}

    Machine(const Machine &rhs) :
      code_stack_(rhs.code_stack_),
      frame_stack_(rhs.frame_stack_),
      obj_stack_(rhs.obj_stack_) {}

    Machine(const Machine &&rhs) :
      Machine(rhs) {}

    Machine(VMCode &ir) :
      code_stack_(),
      frame_stack_(),
      obj_stack_() {
      code_stack_.push_back(&ir);
    }

    void SetPreviousStack(ObjectStack &prev) {
      obj_stack_.SetPreviousStack(prev);
    }

    void Run(bool invoking = false, string id = "", 
      VMCodePointer ptr = nullptr, ObjectMap *p = nullptr, 
      ObjectMap *closure_record = nullptr);
  };

  void InitConsoleComponents();
  void InitBaseTypes();
  void InitContainerComponents();

#if not defined(_DISABLE_SDL_)
  void InitSoundComponents();
#endif

//TODO:Reserved for unix socket wrapper
//TODO: delete macros after finish it
#if defined(_WIN32)
  void LoadSocketStuff();
#endif
}
