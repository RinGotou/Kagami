#pragma once
#include "frontend.h"
#include "management.h"
#include "components.h"

#define CHECK_PRINT_OPT(_Map)                          \
  if (_Map.find(kStrSwitchLine) != p.end()) {          \
    putc('\n', VM_STDOUT);                             \
  }

namespace kagami {
  using Expect = pair<string, string>;
  using ExpectationList = initializer_list<Expect>;
  using NullableList = initializer_list<string>;

  CommentedResult TypeChecking(ExpectationList &&lst,
    ObjectMap &obj_map,
    NullableList &&nullable = {});

#define TC_ERROR(_Obj) Message(std::get<string>(_Obj), kStateError)
#define TC_FAIL(_Obj) !std::get<bool>(_Obj)

  using management::type::PlainComparator;

  PlainType FindTypeCode(string type_id);
  int64_t IntProducer(Object &obj);
  double FloatProducer(Object &obj);
  string StringProducer(Object &obj);
  bool BoolProducer(Object &obj);
  string ParseRawString(const string &src);
  void InitPlainTypesAndConstants();
  void ActivateComponents();
  void ReceiveError(void* vm, const char* msg);

  using ResultTraitKey = pair<PlainType, PlainType>;
  using TraitUnit = pair<ResultTraitKey, PlainType>;
  const map<ResultTraitKey, PlainType> kResultDynamicTraits = {
    TraitUnit(ResultTraitKey(kPlainInt, kPlainInt), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainBool), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainInt), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainBool), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainString, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainInt), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainFloat), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainBool), kPlainString),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainBool), kPlainBool),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainInt), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainString), kPlainString)
  };

  template <typename ResultType, class Tx, class Ty, Keyword op>
  struct BinaryOpBox {
    ResultType Do(Tx A, Ty B) {
      return Tx();
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordPlus> {
    ResultType Do(Tx A, Ty B) {
      return A + B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordMinus> {
    ResultType Do(Tx A, Ty B) {
      return A - B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordTimes> {
    ResultType Do(Tx A, Ty B) {
      return A * B;
    }
  };

  template <typename ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kKeywordDivide> {
    ResultType Do(Tx A, Ty B) {
      return A / B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordEquals> {
    bool Do(Tx A, Ty B) {
      return A == B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordLessOrEqual> {
    bool Do(Tx A, Ty B) {
      return A <= B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordGreaterOrEqual> {
    bool Do(Tx A, Ty B) {
      return A >= B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordNotEqual> {
    bool Do(Tx A, Ty B) {
      return A != B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordGreater> {
    bool Do(Tx A, Ty B) {
      return A > B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordLess> {
    bool Do(Tx A, Ty B) {
      return A < B;
    }
  };

  template <typename Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kKeywordAnd> {
    bool Do(Tx A, Ty B) {
      return A && B;
    }
  };

  template <typename Tx, class Ty>
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

  template <typename ResultType, Keyword op>
  using MathBox = BinaryOpBox<ResultType, ResultType, ResultType, op>;

  template <typename Tx, Keyword op>
  using LogicBox = BinaryOpBox<bool, Tx, Tx, op>;

  const string kIteratorBehavior = "obj|step_forward|__compare";
  const string kContainerBehavior = "head|tail|empty";
  const string kForEachExceptions = "!iterator|!containter_keepalive";

  using CommandPointer = Command * ;
  using EventHandlerMark = pair<Uint32, Uint32>;
  using EventHandler = pair<EventHandlerMark, FunctionImpl>;

  class RuntimeFrame {
  public:
    bool error;
    bool warning;
    bool activated_continue;
    bool activated_break;
    bool void_call;
    bool disable_step;
    bool final_cycle;
    bool jump_from_end;
    bool event_processing;
    bool initializer_calling;
    bool inside_initializer_calling;
    bool stop_point;
    bool has_return_value_from_invoking;
    Object struct_base;
    Object assert_rc_copy;
    size_t jump_offset;
    size_t idx;
    string msg_string;
    string function_scope;
    string struct_id;
    string super_struct_id;
    stack<bool> condition_stack; //preserved
    stack<bool> scope_stack;
    stack<size_t> jump_stack;
    stack<size_t> branch_jump_stack;
    stack<Object> return_stack;

    RuntimeFrame(string scope = kStrRootScope) :
      error(false),
      warning(false),
      activated_continue(false),
      activated_break(false),
      void_call(false),
      disable_step(false),
      final_cycle(false),
      jump_from_end(false),
      event_processing(false),
      initializer_calling(false),
      inside_initializer_calling(false),
      stop_point(false),
      has_return_value_from_invoking(false),
      assert_rc_copy(),
      jump_offset(0),
      idx(0),
      msg_string(),
      function_scope(),
      struct_id(),
      super_struct_id(),
      condition_stack(),
      jump_stack(),
      branch_jump_stack(),
      return_stack() {}

    void Stepping();
    void Goto(size_t taget_idx);
    void AddJumpRecord(size_t target_idx);
    void MakeError(string str);
    void MakeWarning(string str);
    void RefreshReturnStack(Object obj);
  };

  struct _IgnoredException : std::exception {};
  struct _CustomError : std::exception {
  public:
    //TODO:Memory Management
    _CustomError(const char *msg) : 
      std::exception(std::runtime_error(msg)) {}
  };

  class ConfigProcessor {
  private:
    const unordered_map<string, dawn::ImageType> kImageTypeMatcher = {
      make_pair(".jpg", dawn::kImageJPG),
      make_pair(".png", dawn::kImagePNG),
      make_pair(".tif", dawn::kImageTIF),
      make_pair(".tiff", dawn::kImageTIF),
      make_pair(".webp", dawn::kImageWEBP)
    };

  private:
    ObjectStack &obj_stack_;
    stack<RuntimeFrame> &frame_stack_;
    toml::value toml_file_;

  private:
    using TOMLValueTable = unordered_map<string, toml::value>;

    template <typename _Type>
    optional<_Type> ExpectParameter(const toml::value &value, string id) {
      auto expected_value = toml::expect<_Type>(value, id);
      if (expected_value.is_err()) return std::nullopt;
      return expected_value.unwrap();
    }

    void ElementProcessing(ObjectTable &obj_table, string id, 
      const toml::value &elem_def, dawn::PlainWindow &window);
    void TextureProcessing(string id, const toml::value &elem_def, 
      dawn::PlainWindow &window, ObjectTable &table);
    void RectangleProcessing(string id, const toml::value &elem_def,
      ObjectTable &table);
    void InterfaceLayoutProcessing(string target_elem_id, 
      const toml::value &elem_def, dawn::PlainWindow &window);
  public:
    ConfigProcessor() = delete;
    ConfigProcessor(ObjectStack &obj_stack, stack<RuntimeFrame> &frames, string file) noexcept :
      obj_stack_(obj_stack), frame_stack_(frames), toml_file_() {
      try { toml_file_ = toml::parse(file); }
      catch (std::runtime_error &e) {
        frame_stack_.top().MakeError(e.what());
      }
      catch (toml::syntax_error &e) {
        frame_stack_.top().MakeError(e.what());
      }
    }

    string GetTableVariant();
    void InitWindowFromConfig();
    void InitTextureTable(ObjectTable &table, dawn::PlainWindow &window);
    void InitRectangleTable(ObjectTable &table);
    void ApplyInterfaceLayout(dawn::PlainWindow &window);
  };

  class Machine {
  private:
    StandardLogger *logger_;
    bool is_logger_host_;

  private:
    void RecoverLastState();
    void FinishInitalizerCalling();
    bool IsTailRecursion(size_t idx, VMCode *code);
    bool IsTailCall(size_t idx);

    Object FetchPlainObject(Argument &arg);
    Object FetchFunctionObject(string id);
    Object FetchObject(Argument &arg, bool checking = false);

    bool FetchFunctionImplEx(FunctionImplPointer &dest, string id, string type_id = kTypeIdNull, 
      Object *obj_ptr = nullptr);

    bool FetchFunctionImpl(FunctionImplPointer &impl, CommandPointer &command,
      ObjectMap &obj_map);

    void ClosureCatching(ArgumentList &args, size_t nest_end, bool closure);

    Message CallMethod(Object obj, string id, ObjectMap &args);
    Message CallMethod(Object obj, string id,
      const initializer_list<NamedObject> &&args = {});
    Message CallVMCFunction(FunctionImpl &impl, ObjectMap &obj_map);

    void CommandIfOrWhile(Keyword token, ArgumentList &args, size_t nest_end);
    void CommandForEach(ArgumentList &args, size_t nest_end);
    void ForEachChecking(ArgumentList &args, size_t nest_end);
    void CommandCase(ArgumentList &args, size_t nest_end);
    void CommandElse();
    void CommandWhen(ArgumentList &args);
    void CommandContinueOrBreak(Keyword token, size_t escape_depth);
    void CommandStructBegin(ArgumentList &args);
    void CommandModuleBegin(ArgumentList &args);
    void CommandConditionEnd();
    void CommandLoopEnd(size_t nest);
    void CommandForEachEnd(size_t nest);
    void CommandStructEnd();
    void CommandModuleEnd();
    void CommandInclude(ArgumentList &args);
    void CommandSuper(ArgumentList &args);

    void CommandHash(ArgumentList &args);
    void CommandSwap(ArgumentList &args);
    void CommandBind(ArgumentList &args, bool local_value, bool ext_value);
    void CommandDelivering(ArgumentList &args, bool local_value, bool ext_value);
    void CommandTypeId(ArgumentList &args);
    void CommandMethods(ArgumentList &args);
    void CommandExist(ArgumentList &args);
    void CommandNullObj(ArgumentList &args);
    void CommandDestroy(ArgumentList &args);
    void CommandConvert(ArgumentList &args);
    void CommandUsing(ArgumentList &args);
    void CommandUsingTable(ArgumentList &args);
    void CommandApplyLayout(ArgumentList &args);
    void CommandOffensiveMode(ArgumentList &args);

    void CommandTime();
    void CommandVersion();
    void CommandMachineCodeName();

    template <Keyword op_code>
    void BinaryMathOperatorImpl(ArgumentList &args);

    template <Keyword op_code>
    void BinaryLogicOperatorImpl(ArgumentList &args);

    void OperatorLogicNot(ArgumentList &args);

    void ExpList(ArgumentList &args);
    void InitArray(ArgumentList &args);

    void CommandReturn(ArgumentList &args);
    void CommandAssert(ArgumentList &args);
    void CommandHandle(ArgumentList &args);
    void CommandWait(ArgumentList &args);
    void CommandLeave(ArgumentList &args);
    void DomainAssert(ArgumentList &args);

    void CommandIsBaseOf(ArgumentList &args);
    void CommandHasBehavior(ArgumentList &args);
    template <ParameterPattern pattern>
    void CommandCheckParameterPattern(ArgumentList &args);
    void CommandOptionalParamRange(ArgumentList &args);

    void MachineCommands(Keyword token, ArgumentList &args, Request &request);

    void GenerateArgs(FunctionImpl &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_Fixed(FunctionImpl &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoSize(FunctionImpl &impl, ArgumentList &args, ObjectMap &obj_map);
    void Generate_AutoFill(FunctionImpl &impl, ArgumentList &args, ObjectMap &obj_map);
    void LoadEventInfo(SDL_Event &event, ObjectMap &obj_map, FunctionImpl &impl, Uint32 id);
    void CallExtensionFunction(ObjectMap &p, FunctionImpl &impl);
    //void CallExtensionFunctionEx

    void GenerateStructInstance(ObjectMap &p);

    void GenerateErrorMessages(size_t stop_index);
  private:
    deque<VMCodePointer> code_stack_;
    stack<RuntimeFrame> frame_stack_;
    ObjectStack obj_stack_;
    map<EventHandlerMark, FunctionImpl> event_list_;
    bool hanging_;
    bool freezing_;
    bool error_;
    bool offensive_;

  public:
    ~Machine() { if (is_logger_host_) delete logger_; }
    Machine() = delete;
    Machine(const Machine &rhs) = delete;
    Machine(const Machine &&rhs) = delete;
    void operator=(const Machine &) = delete;
    void operator=(const Machine &&) = delete;

    Machine(VMCode &ir, string log_path, bool rtlog = false) :
      logger_(nullptr),
      is_logger_host_(true),
      code_stack_(),
      frame_stack_(),
      obj_stack_(),
      event_list_(), 
      hanging_(false), 
      freezing_(false),
      error_(false),
      offensive_(false) { 

      code_stack_.push_back(&ir); 
      logger_ = rtlog ?
        (StandardLogger *)new StandardRTLogger(log_path, "a") :
        (StandardLogger *)new StandardCachedLogger(log_path, "a");
    }

    Machine(VMCode &ir, StandardLogger *logger) :
      logger_(logger),
      is_logger_host_(false),
      code_stack_(),
      frame_stack_(),
      obj_stack_(),
      event_list_(),
      hanging_(false),
      freezing_(false),
      error_(false),
      offensive_(false) {

      code_stack_.push_back(&ir);
    }

    bool PushObject(string id, Object object);
    void PushError(string msg);

    void SetPreviousStack(ObjectStack &prev) {
      obj_stack_.SetPreviousStack(prev);
    }

    void SetDelegatedRoot(ObjectContainer &root) {
      obj_stack_.SetDelegatedRoot(root);
    }

    void Run(bool invoke = false);

    bool ErrorOccurred() const {
      return error_;
    }
  };
}
