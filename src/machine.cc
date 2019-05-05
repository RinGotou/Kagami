#include "machine.h"

#define ERROR_CHECKING(_Cond, _Msg) if (_Cond) { frame.MakeError(_Msg); return; }

namespace kagami {
  using namespace management;

  PlainType FindTypeCode(string type_id) {
    auto it = kTypeStore.find(type_id);
    return it != kTypeStore.end() ? it->second : kNotPlainType;
  }

  int64_t IntProducer(Object &obj) {
    int64_t result = 0;
    switch (auto type = FindTypeCode(obj.GetTypeId()); type) {
    case kPlainInt:result = obj.Cast<int64_t>(); break;
    case kPlainFloat:result = static_cast<int64_t>(obj.Cast<double>()); break;
    case kPlainBool:result = obj.Cast<bool>() ? 1 : 0; break;
    default:break;
    }

    return result;
  }

  double FloatProducer(Object &obj) {
    double result = 0;
    switch (auto type = FindTypeCode(obj.GetTypeId()); type) {
    case kPlainFloat:result = obj.Cast<double>(); break;
    case kPlainInt:result = static_cast<double>(obj.Cast<int64_t>()); break;
    case kPlainBool:result = obj.Cast<bool>() ? 1.0 : 0.0; break;
    default:break;
    }

    return result;
  }

  string StringProducer(Object &obj) {
    string result;
    switch (auto type = FindTypeCode(obj.GetTypeId()); type) {
    case kPlainInt:result = to_string(obj.Cast<int64_t>()); break;
    case kPlainFloat:result = to_string(obj.Cast<double>()); break;
    case kPlainBool:result = obj.Cast<bool>() ? kStrTrue : kStrFalse; break;
    case kPlainString:result = obj.Cast<string>(); break;
    default:break;
    }

    return result;
  }

  bool BoolProducer(Object &obj) {
    auto it = kTypeStore.find(obj.GetTypeId());
    auto type = it != kTypeStore.end() ? it->second : kNotPlainType;
    bool result = false;

    if (type == kPlainInt) {
      int64_t value = obj.Cast<int64_t>();
      if (value > 0) result = true;
    }
    else if (type == kPlainFloat) {
      double value = obj.Cast<double>();
      if (value > 0.0) result = true;
    }
    else if (type == kPlainBool) {
      result = obj.Cast<bool>();
    }
    else if (type == kPlainString) {
      string &value = obj.Cast<string>();
      result = !value.empty();
    }

    return result;
  }

  /* string/wstring convertor */
  //from https://www.yasuhisay.info/interface/20090722/1248245439
  std::wstring s2ws(const std::string &s) {
    if (s.empty()) return wstring();
    size_t length = s.size();
    wchar_t *wc = (wchar_t *)malloc(sizeof(wchar_t) * (length + 2));
    mbstowcs(wc, s.data(), s.length() + 1);
    std::wstring str(wc);
    free(wc);
    return str;
  }

  std::string ws2s(const std::wstring & s) {
    if (s.empty()) return string();
    size_t length = s.size();
    char *c = (char *)malloc(sizeof(char) * length * 2);
    wcstombs(c, s.data(), s.length() + 1);
    std::string result(c);
    free(c);
    return result;
  }
  string ParseRawString(const string & src) {
    string result = src;
    if (util::IsString(result)) result = util::GetRawString(result);
    return result;
  }

  void InitPlainTypes() {
    using type::NewTypeSetup;
    using type::PlainHasher;

    NewTypeSetup(kTypeIdInt, SimpleSharedPtrCopy<int64_t>, PlainHasher<int64_t>())
      .InitComparator(PlainComparator<int64_t>);
    NewTypeSetup(kTypeIdFloat, SimpleSharedPtrCopy<double>, PlainHasher<double>())
      .InitComparator(PlainComparator<double>);
    NewTypeSetup(kTypeIdBool, SimpleSharedPtrCopy<bool>, PlainHasher<bool>())
      .InitComparator(PlainComparator<bool>);
    NewTypeSetup(kTypeIdNull, FakeCopy<void>);

    EXPORT_CONSTANT(kTypeIdInt);
    EXPORT_CONSTANT(kTypeIdFloat);
    EXPORT_CONSTANT(kTypeIdBool);
    EXPORT_CONSTANT(kTypeIdNull);
  }

  void RuntimeFrame::Steping() {
    if (!disable_step) idx += 1;
    disable_step = false;
  }

  void RuntimeFrame::Goto(size_t target_idx) {
    idx = target_idx - jump_offset;
    disable_step = true;
  }

  void RuntimeFrame::AddJumpRecord(size_t target_idx) {
    if (jump_stack.empty() || jump_stack.top() != target_idx) {
      jump_stack.push(target_idx);
    }
  }

  void RuntimeFrame::MakeError(string str) {
    error = true;
    error_string = str;
  }

  void RuntimeFrame::RefreshReturnStack(Object obj) {
    if (!void_call) {
      return_stack.push(std::move(obj));
    }
  }

  void Machine::RecoverLastState() {
    frame_stack_.pop();
    code_stack_.pop_back();
    obj_stack_.Pop();
  }

  Object Machine::FetchPlainObject(Argument &arg) {
    auto type = arg.token_type;
    auto &value = arg.data;
    Object obj;

    if (type == kStringTypeInt) {
      int64_t int_value;
      from_chars(value.data(), value.data() + value.size(), int_value);
      obj.PackContent(make_shared<int64_t>(int_value), kTypeIdInt);
    }
    else if (type == kStringTypeFloat) {
      double float_value;
#if not defined (_MSC_VER)
      float_value = stod(value);
#else
      from_chars(value.data(), value.data() + value.size(), float_value);
#endif
      obj.PackContent(make_shared<double>(float_value), kTypeIdFloat);
    }
    else {
      switch (type) {
      case kStringTypeBool:
        obj.PackContent(make_shared<bool>(value == kStrTrue), kTypeIdBool);
        break;
      case kStringTypeString:
        obj.PackContent(make_shared<string>(ParseRawString(value)), kTypeIdString);
        break;
      case kStringTypeIdentifier:
        obj.PackContent(make_shared<string>(value), kTypeIdString);
        break;
      default:
        break;
      }
    }

    return obj;
  }

  Object Machine::FetchInterfaceObject(string id) {
    Object obj;
    auto &frame = frame_stack_.top();
    
    if (auto ptr = FindInterface(id); ptr != nullptr) {
      auto interface = *ptr;
      obj.PackContent(make_shared<Interface>(interface), kTypeIdFunction);
    }

    return obj;
  }

  Object Machine::FetchObject(Argument &arg, bool checking) {
    if (arg.type == kArgumentNormal) {
      return FetchPlainObject(arg);
    }

    auto &frame = frame_stack_.top();
    auto &return_stack = frame.return_stack;
    ObjectPointer ptr = nullptr;
    Object obj;

    if (arg.type == kArgumentObjectStack) {
      if (ptr = obj_stack_.Find(arg.data); ptr != nullptr) {
        obj.PackObject(*ptr);
        return obj;
      }

      if (obj = GetConstantObject(arg.data); obj.Null()) {
        obj = FetchInterfaceObject(arg.data);
      }

      if (obj.Null()) {
        frame.MakeError("Object is not found - " + arg.data);
      }
    }
    else if (arg.type == kArgumentReturnStack) {
      if (!return_stack.empty()) {
        obj = return_stack.top();
        if(!checking) return_stack.pop(); 
      }
      else {
        frame.MakeError("Can't get object from stack.");
      }
    }

    return obj;
  }

  bool Machine::_FetchInterface(InterfacePointer &interface, string id, string type_id) {
    auto &frame = frame_stack_.top();

    //Modified version for function invoking
    if (type_id != kTypeIdNull) {
      if (interface = FindInterface(id, type_id); interface == nullptr) {
        frame.MakeError("Method is not found - " + id);
        return false;
      }

      return true;
    }
    else {
      if (interface = FindInterface(id); interface != nullptr) return true;
      ObjectPointer ptr = obj_stack_.Find(id);
      if (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction) {
        interface = &ptr->Cast<Interface>();
        return true;
      }

      frame.MakeError("Function is not found - " + id);
    }

    return false;
  }

  bool Machine::FetchInterface(InterfacePointer &interface, CommandPointer &command, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    auto id = command->first.GetInterfaceId();
    auto domain = command->first.GetInterfaceDomain();
    

    //Object methods.
    //In current developing processing, machine forced to querying built-in
    //function. These code need to be rewritten when I work in class feature in
    //the future.
    if (domain.type != kArgumentNull) {
      Object obj = FetchObject(domain, true);

      if (frame.error) return false;

      if (interface = FindInterface(id, obj.GetTypeId()); interface == nullptr) {
        frame.MakeError("Method is not found - " + id);
        return false;
      }

      obj_map.emplace(NamedObject(kStrMe, obj));
      return true;
    }
    //Plain bulit-in function and user-defined function
    //At first, Machine will querying in built-in function map,
    //and then try to fetch function object in heap.
    else {
      if (interface = FindInterface(id); interface != nullptr) return true;

      ObjectPointer ptr = obj_stack_.Find(id);

      if (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction) {
        interface = &ptr->Cast<Interface>();
        return true;
      }

      frame.MakeError("Function is not found - " + id);
    }

    return false;
  }

  void Machine::ClosureCatching(ArgumentList &args, size_t nest_end, bool closure) {
    auto &frame = frame_stack_.top();
    auto &obj_list = obj_stack_.GetBase();
    auto &origin_code = *code_stack_.back();
    size_t counter = 0, size = args.size(), nest = frame.idx;
    bool optional = false, variable = false;
    StateCode argument_mode = kCodeNormalParam;
    vector<string> params;
    VMCode code;

    for (size_t idx = nest + 1; idx < nest_end - frame.jump_offset; ++idx) {
      code.push_back(origin_code[idx]);
    }

    for (size_t idx = 1; idx < size; idx += 1) {
      auto &id = args[idx].data;

      if (id == kStrOptional) {
        optional = true;
        counter += 1;
        continue;
      }

      if (id == kStrVariable) {
        if (counter == 1) {
          frame.MakeError("Variable parameter can be defined only once");
          break;
        }

        if (idx != size - 2) {
          frame.MakeError("Variable parameter must be last one");
          break;
        }

        variable = true;
        counter += 1;
        continue;
      }

      if (optional && args[idx - 1].data != kStrOptional) {
        frame.MakeError("Optional parameter must be defined after normal parameters");
      }

      params.push_back(id);
    }

    if (optional && variable) {
      frame.MakeError("Variable and optional parameter can't be defined at same time");
      return;
    }

    if (optional) argument_mode = kCodeAutoFill;
    if (variable) argument_mode = kCodeAutoSize;

    Interface interface(nest + 1, code, args[0].data, params, argument_mode);

    if (optional) {
      interface.SetMinArgSize(params.size() - counter);
    }

    if (closure) {
      ObjectMap scope_record;
      auto &base = obj_stack_.GetBase();
      auto it = base.rbegin();
      bool flag = false;

      for (; it != base.rend(); ++it) {
        if (flag) break;

        if (it->Find(kStrUserFunc) != nullptr) flag = true;

        for (auto &unit : it->GetContent()) {
          if (scope_record.find(unit.first) == scope_record.end()) {
            scope_record.insert(NamedObject(unit.first,
              type::CreateObjectCopy(unit.second)));
          }
        }
      }

      interface.SetClosureRecord(scope_record);
    }

    obj_stack_.CreateObject(args[0].data,
      Object(make_shared<Interface>(interface), kTypeIdFunction));

    frame.Goto(nest_end + 1);
  }

  Message Machine::Invoke(Object obj, string id, const initializer_list<NamedObject> &&args) {
    InterfacePointer interface;

    if (bool found = _FetchInterface(interface, id, obj.GetTypeId()); !found) {
      //Immediately push event to avoid ugly checking block.
      trace::AddEvent("Method is not found - " + id);
      return Message();
    }

    ObjectMap obj_map = args;
    obj_map.insert(NamedObject(kStrMe, obj));

    if (interface->GetPolicyType() == kInterfaceVMCode) {
      Run(true, id, &interface->GetCode(), &obj_map, &interface->GetClosureRecord());
      Object obj = frame_stack_.top().return_stack.top();
      frame_stack_.top().return_stack.pop();
      return Message().SetObject(obj);
    }

    return interface->Start(obj_map);
  }

  void Machine::CommandIfOrWhile(Keyword token, ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    auto &code = code_stack_.front();
    REQUIRED_ARG_COUNT(1);

    if (token == kKeywordIf || token == kKeywordWhile) {
      frame.AddJumpRecord(nest_end);
      code->FindJumpRecord(frame.idx + frame.jump_offset, frame.branch_jump_stack);
    }
    
    Object obj = FetchObject(args[0]);
    ERROR_CHECKING(obj.GetTypeId() != kTypeIdBool, "Invalid state value type.");

    bool state = obj.Cast<bool>();

    if (token == kKeywordIf) {
      frame.scope_stack.push(false);
      frame.condition_stack.push(state);
      if (!state) {
        if (frame.branch_jump_stack.empty()) {
          frame.Goto(frame.jump_stack.top());
        }
        else {
          frame.Goto(frame.branch_jump_stack.top());
          frame.branch_jump_stack.pop();
        }
      }
    }
    else if (token == kKeywordElif) {
      ERROR_CHECKING(frame.condition_stack.empty(), "Unexpected Elif.");

      if (frame.condition_stack.top()) {
        frame.Goto(frame.jump_stack.top());
      }
      else {
        if (state) {
          frame.condition_stack.top() = true;
        }
        else {
          if (frame.branch_jump_stack.empty()) {
            frame.Goto(frame.jump_stack.top());
          }
          else {
            frame.Goto(frame.branch_jump_stack.top());
            frame.branch_jump_stack.pop();
          }
        }
      }
    }
    else if (token == kKeywordWhile) {
      if (!frame.jump_from_end) {
        frame.scope_stack.push(true);
        obj_stack_.Push();
      }
      else {
        frame.jump_from_end = false;
      }

      if (!state) {
        frame.Goto(nest_end);
        frame.final_cycle = true;
      }
    }
  }

  void Machine::CommandForEach(ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    ObjectMap obj_map;

    frame.AddJumpRecord(nest_end);

    if (frame.jump_from_end) {
      ForEachChecking(args, nest_end);
      frame.jump_from_end = false;
      return;
    }

    auto unit_id = FetchObject(args[0]).Cast<string>();
    auto container_obj = FetchObject(args[1]);
    ERROR_CHECKING(!type::CheckBehavior(container_obj, kContainerBehavior),
      "Invalid object container");

    auto msg = Invoke(container_obj, kStrHead);
    ERROR_CHECKING(msg.GetCode() != kCodeObject,
      "Invalid iterator of container");

    auto iterator_obj = msg.GetObj();
    ERROR_CHECKING(!type::CheckBehavior(iterator_obj, kIteratorBehavior),
      "Invalid iterator behavior");

    auto unit = Invoke(iterator_obj, "obj").GetObj();

    frame.scope_stack.push(true);
    obj_stack_.Push();
    obj_stack_.CreateObject(kStrIteratorObj, iterator_obj);
    obj_stack_.CreateObject(unit_id, unit);
  }

  void Machine::ForEachChecking(ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    auto unit_id = FetchObject(args[0]).Cast<string>();
    auto iterator = *obj_stack_.GetCurrent().Find(kStrIteratorObj);
    auto container = FetchObject(args[1]);
    ObjectMap obj_map;

    auto tail = Invoke(container, kStrTail).GetObj();
    ERROR_CHECKING(!type::CheckBehavior(tail, kIteratorBehavior),
      "Invalid container behavior");

    Invoke(iterator, "step_forward");

    auto result = Invoke(iterator, kStrCompare,
      { NamedObject(kStrRightHandSide,tail) }).GetObj();
    ERROR_CHECKING(result.GetTypeId() != kTypeIdBool,
      "Invalid iterator behavior");

    if (result.Cast<bool>()) {
      frame.Goto(nest_end);
      frame.final_cycle = true;
    }
    else {
      auto unit = Invoke(iterator, "obj").GetObj();
      obj_stack_.CreateObject(unit_id, unit);
    }
  }

  void Machine::CommandCase(ArgumentList &args, size_t nest_end) {
    auto &frame = frame_stack_.top();
    auto &code = code_stack_.front();
    ERROR_CHECKING(args.empty(), "Empty argument list");
    frame.AddJumpRecord(nest_end);

    bool has_jump_list = 
      code->FindJumpRecord(frame.idx + frame.jump_offset, frame.branch_jump_stack);

    Object obj = FetchObject(args[0]);
    string type_id = obj.GetTypeId();
    ERROR_CHECKING(!util::IsPlainType(type_id),
      "Non-plain object is not supported by case");

    Object sample_obj = type::CreateObjectCopy(obj);

    frame.scope_stack.push(true);
    obj_stack_.Push();
    obj_stack_.CreateObject(kStrCaseObj, sample_obj);
    frame.condition_stack.push(false);

    if (has_jump_list) {
      frame.Goto(frame.branch_jump_stack.top());
      frame.branch_jump_stack.pop();
    }
    else {
      //although I think no one will write case block without condition branch...
      frame.Goto(frame.jump_stack.top());
    }
  }

  void Machine::CommandElse() {
    auto &frame = frame_stack_.top();
    ERROR_CHECKING(frame.condition_stack.empty(), "Unexpected Else.");

    if (frame.condition_stack.top() == true) {
      frame.Goto(frame.jump_stack.top());
    }
    else {
      frame.condition_stack.top() = true;
    }
  }

  void Machine::CommandWhen(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    bool result = false;
    ERROR_CHECKING(frame.condition_stack.empty(), 
      "Unexpected 'when'");

    if (frame.condition_stack.top()) {
      frame.Goto(frame.jump_stack.top());
      return;
    }

    if (!args.empty()) {
      ObjectPointer ptr = obj_stack_.Find(kStrCaseObj);
      string type_id = ptr->GetTypeId();
      bool found = false;

      ERROR_CHECKING(ptr == nullptr, 
        "Unexpected 'when'");
      ERROR_CHECKING(!util::IsPlainType(type_id), 
        "Non-plain object is not supported by when");

#define COMPARE_RESULT(_Type) (ptr->Cast<_Type>() == obj.Cast<_Type>())

      for (auto it = args.rbegin(); it != args.rend(); ++it) {
        Object obj = FetchObject(*it);

        if (obj.GetTypeId() != type_id) continue;

        if (type_id == kTypeIdInt) {
          found = COMPARE_RESULT(int64_t);
        }
        else if (type_id == kTypeIdFloat) {
          found = COMPARE_RESULT(double);
        }
        else if (type_id == kTypeIdString) {
          found = COMPARE_RESULT(string);
        }
        else if (type_id == kTypeIdBool) {
          found = COMPARE_RESULT(bool);
        }

        if (found) break;
      }
#undef COMPARE_RESULT

      if (found) {
        frame.condition_stack.top() = true;
      }
      else {
        if (!frame.branch_jump_stack.empty()) {
          frame.Goto(frame.branch_jump_stack.top());
          frame.branch_jump_stack.pop();
        }
        else {
          frame.Goto(frame.jump_stack.top());
        }
      }
    }
  }

  void Machine::CommandContinueOrBreak(Keyword token, size_t escape_depth) {
    auto &frame = frame_stack_.top();
    auto &scope_stack = frame.scope_stack;

    while (escape_depth != 0) {
      frame.condition_stack.pop();
      frame.jump_stack.pop();
      if (!scope_stack.empty() && scope_stack.top()) {
        obj_stack_.Pop();
      }
      scope_stack.pop();
      escape_depth -= 1;
    }

    frame.Goto(frame.jump_stack.top());

    switch (token) {
    case kKeywordContinue:
      frame.activated_continue = true; 
      break;
    case kKeywordBreak:
      frame.activated_break = true; 
      frame.final_cycle = true;
      break;
    default:break;
    }
  }

  void Machine::CommandConditionEnd() {
    auto &frame = frame_stack_.top();
    frame.condition_stack.pop();
    frame.jump_stack.pop();
    frame.scope_stack.pop();
    while (!frame.branch_jump_stack.empty()) frame.branch_jump_stack.pop();
  }

  void Machine::CommandLoopEnd(size_t nest) {
    auto &frame = frame_stack_.top();

    if (frame.final_cycle) {
      if (frame.activated_continue) {
        frame.Goto(nest);
        frame.activated_continue = false;
        obj_stack_.GetCurrent().clear();
        frame.jump_from_end = true;
      }
      else {
        if (frame.activated_break) frame.activated_break = false;
        while (!frame.return_stack.empty()) frame.return_stack.pop();
        frame.jump_stack.pop();
        obj_stack_.Pop();
      }
      frame.scope_stack.pop();
      frame.final_cycle = false;
    }
    else {
      frame.Goto(nest);
      while (!frame.return_stack.empty()) frame.return_stack.pop();
      obj_stack_.GetCurrent().clear();
      frame.jump_from_end = true;
    }
  }

  void Machine::CommandForEachEnd(size_t nest) {
    auto &frame = frame_stack_.top();

    if (frame.final_cycle) {
      if (frame.activated_continue) {
        frame.Goto(nest);
        frame.activated_continue = false;
        obj_stack_.GetCurrent().ClearExcept(kStrIteratorObj);
        frame.jump_from_end = true;
      }
      else {
        if (frame.activated_break) frame.activated_break = false;
        frame.jump_stack.pop();
        obj_stack_.Pop();
      }
      frame.scope_stack.pop();
      frame.final_cycle = false;
    }
    else {
      frame.Goto(nest);
      obj_stack_.GetCurrent().ClearExcept(kStrIteratorObj);
      frame.jump_from_end = true;
    }
  }

  void Machine::CommandHash(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    auto &obj = FetchObject(args[0]).Unpack();

    if (type::IsHashable(obj)) {
      int64_t hash = type::GetHash(obj);
      frame.RefreshReturnStack(Object(make_shared<int64_t>(hash), kTypeIdInt));
    }
    else {
      frame.RefreshReturnStack(Object());
    }
  }

  void Machine::CommandSwap(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    auto &right = FetchObject(args[1]).Unpack();
    auto &left = FetchObject(args[0]).Unpack();

    left.swap(right);
  }

  void Machine::CommandBind(ArgumentList &args, bool local_value) {
    using namespace type;
    auto &frame = frame_stack_.top();
    //Do not change the order!
    auto rhs = FetchObject(args[1]);
    auto lhs = FetchObject(args[0]);

    if (lhs.IsRef()) {
      auto &real_lhs = lhs.Unpack();
      real_lhs = CreateObjectCopy(rhs);
      return;
    }
    else {
      string id = lhs.Cast<string>();

      if (!local_value) {
        ObjectPointer ptr = obj_stack_.Find(id);

        if (ptr != nullptr) {
          ptr->Unpack() = CreateObjectCopy(rhs);
          return;
        }
      }

      Object obj = CreateObjectCopy(rhs);
      ERROR_CHECKING(util::GetStringType(id) != kStringTypeIdentifier,
        "Invalid object id.");
      ERROR_CHECKING(!obj_stack_.CreateObject(id, obj),
        "Object binding failed.");
    }
  }

  void Machine::CommandTypeId(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    if (args.size() > 1) {
      ManagedArray base = make_shared<ObjectArray>();

      for (auto &unit : args) {
        base->emplace_back(Object(FetchObject(unit).GetTypeId()));
      }

      Object obj(base, kTypeIdArray);
      obj.SetConstructorFlag();
      frame.RefreshReturnStack(obj);
    }
    else if (args.size() == 1) {
      frame.RefreshReturnStack(Object(FetchObject(args[0]).GetTypeId()));
    }
    else {
      frame.RefreshReturnStack(Object(kTypeIdNull));
    }
  }

  void Machine::CommandMethods(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    REQUIRED_ARG_COUNT(1);

    Object obj = FetchObject(args[0]);
    auto methods = type::GetMethods(obj.GetTypeId());
    ManagedArray base = make_shared<ObjectArray>();

    for (auto &unit : methods) {
      base->emplace_back(Object(unit, kTypeIdString));
    }

    Object ret_obj(base, kTypeIdArray);
    ret_obj.SetConstructorFlag();
    frame.RefreshReturnStack(ret_obj);
  }

  void Machine::CommandExist(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    REQUIRED_ARG_COUNT(2);

    //Do not change the order
    auto str_obj = FetchObject(args[1]);
    auto obj = FetchObject(args[0]);
    ERROR_CHECKING(str_obj.GetTypeId() != kTypeIdString, "Invalid method id");

    string str = str_obj.Cast<string>();
    Object ret_obj(type::CheckMethod(str, obj.GetTypeId()), kTypeIdBool);

    frame.RefreshReturnStack(ret_obj);
  }

  void Machine::CommandNullObj(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    REQUIRED_ARG_COUNT(1);

    Object obj = FetchObject(args[0]);
    frame.RefreshReturnStack(Object(obj.GetTypeId() == kTypeIdNull, kTypeIdBool));
  }

  void Machine::CommandDestroy(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    REQUIRED_ARG_COUNT(1);

    Object &obj = FetchObject(args[0]).Unpack();
    obj.swap(Object());
  }

  void Machine::CommandConvert(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    REQUIRED_ARG_COUNT(1);

    auto &arg = args[0];
    if (arg.type == kArgumentNormal) {
      FetchPlainObject(arg);
    }
    else {
      Object obj = FetchObject(args[0]);
      string type_id = obj.GetTypeId();
      Object ret_obj;

      if (type_id == kTypeIdString) {
        auto str = obj.Cast<string>();
        auto type = util::GetStringType(str, true);

        switch (type) {
        case kStringTypeInt:
          ret_obj.PackContent(make_shared<int64_t>(stol(str)), kTypeIdInt);
          break;
        case kStringTypeFloat:
          ret_obj.PackContent(make_shared<double>(stod(str)), kTypeIdFloat);
          break;
        case kStringTypeBool:
          ret_obj.PackContent(make_shared<bool>(str == kStrTrue), kTypeIdBool);
          break;
        default:
          ret_obj = obj;
          break;
        }
      }
      else {
        ERROR_CHECKING(!type::CheckMethod(kStrGetStr, type_id),
          "Invalid argument of convert()");

        auto ret_obj = Invoke(obj, kStrGetStr).GetObj();
      }

      frame.RefreshReturnStack(ret_obj);
    }
  }

  void Machine::CommandRefCount(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    REQUIRED_ARG_COUNT(1);

    auto &obj = FetchObject(args[0]).Unpack();
    Object ret_obj(make_shared<int64_t>(obj.ObjRefCount()), kTypeIdInt);

    frame.RefreshReturnStack(ret_obj);
  }

  void Machine::CommandTime() {
    auto &frame = frame_stack_.top();
    time_t now = time(nullptr);
    string nowtime(ctime(&now));
    nowtime.pop_back();
    frame.RefreshReturnStack(Object(nowtime));
  }

  void Machine::CommandVersion() {
    auto &frame = frame_stack_.top();
    frame.RefreshReturnStack(Object(kInterpreterVersion));
  }

  void Machine::CommandMachineCodeName() {
    auto &frame = frame_stack_.top();
    frame.RefreshReturnStack(Object(kCodeName));
  }

  template <Keyword op_code>
  void Machine::BinaryMathOperatorImpl(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    REQUIRED_ARG_COUNT(2);
    auto rhs = FetchObject(args[1]);
    auto lhs = FetchObject(args[0]);
    auto type_rhs = FindTypeCode(rhs.GetTypeId());
    auto type_lhs = FindTypeCode(lhs.GetTypeId());

    if (type_rhs == kNotPlainType || type_rhs == kNotPlainType) {
      frame.RefreshReturnStack();
      return;
    }

    auto result_type = kResultDynamicTraits.at(ResultTraitKey(type_lhs, type_rhs));

#define RESULT_PROCESSING(_Type, _Func, _TypeId)                       \
  _Type result = MathBox<_Type, op_code>().Do(_Func(lhs), _Func(rhs)); \
  frame.RefreshReturnStack(Object(result, _TypeId));

    if (result_type == kPlainString) {
      if (!find_in_vector(op_code, kStringOpStore)) {
        frame.RefreshReturnStack();
        return;
      }

      RESULT_PROCESSING(string, StringProducer, kTypeIdString);
    }
    else if (result_type == kPlainInt) {
      RESULT_PROCESSING(int64_t, IntProducer, kTypeIdInt);
    }
    else if (result_type == kPlainFloat) {
      RESULT_PROCESSING(double, FloatProducer, kTypeIdFloat);
    }
    else if (result_type == kPlainBool) {
      RESULT_PROCESSING(bool, BoolProducer, kTypeIdBool);
    }
#undef RESULT_PROCESSING
  }

  template <Keyword op_code>
  void Machine::BinaryLogicOperatorImpl(ArgumentList &args) {
    using namespace type;
    auto &frame = frame_stack_.top();

    REQUIRED_ARG_COUNT(2);

    auto rhs = FetchObject(args[1]);
    auto lhs = FetchObject(args[0]);
    auto type_rhs = FindTypeCode(rhs.GetTypeId());
    auto type_lhs = FindTypeCode(lhs.GetTypeId());
    bool result = false;

    if (!util::IsPlainType(lhs.GetTypeId())) {
      if (op_code != kKeywordEquals && op_code != kKeywordNotEqual) {
        frame.RefreshReturnStack();
        return;
      }

      if (!CheckMethod(kStrCompare, lhs.GetTypeId())) {
        frame.MakeError("Can't operate with this operator.");
        return;
      }

      Object obj = Invoke(lhs, kStrCompare,
        { NamedObject(kStrRightHandSide, rhs) }).GetObj();

      if (obj.GetTypeId() != kTypeIdBool) {
        frame.MakeError("Invalid behavior of compare().");
        return;
      }

      if (op_code == kKeywordNotEqual) {
        bool value = !obj.Cast<bool>();
        frame.RefreshReturnStack(Object(value, kTypeIdBool));
      }
      else {
        frame.RefreshReturnStack(obj);
      }
      return;
    }

    auto result_type = kResultDynamicTraits.at(ResultTraitKey(type_lhs, type_rhs));
#define RESULT_PROCESSING(_Type, _Func)\
  result = LogicBox<_Type, op_code>().Do(_Func(lhs), _Func(rhs));

    if (result_type == kPlainString) {
      if (!find_in_vector(op_code, kStringOpStore)) {
        frame.RefreshReturnStack();
        return;
      }

      RESULT_PROCESSING(string, StringProducer);
    }
    else if (result_type == kPlainInt) {
      RESULT_PROCESSING(int64_t, IntProducer);
    }
    else if (result_type == kPlainFloat) {
      RESULT_PROCESSING(double, FloatProducer);
    }
    else if (result_type == kPlainBool) {
      RESULT_PROCESSING(bool, BoolProducer);
    }

    frame.RefreshReturnStack(Object(result, kTypeIdBool));
#undef RESULT_PROCESSING
  }

  void Machine::OperatorLogicNot(ArgumentList &args) {
    auto &frame = frame_stack_.top();

    REQUIRED_ARG_COUNT(1);

    auto rhs = FetchObject(args[0]);

    if (rhs.GetTypeId() != kTypeIdBool) {
      frame.MakeError("Can't operate with this operator");
      return;
    }

    bool result = !rhs.Cast<bool>();

    frame.RefreshReturnStack(Object(result, kTypeIdBool));
  }


  void Machine::ExpList(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    if (!args.empty()) {
      frame.RefreshReturnStack(FetchObject(args.back()));
    }
  }

  void Machine::InitArray(ArgumentList &args) {
    auto &frame = frame_stack_.top();
    ManagedArray base = make_shared<ObjectArray>();

    if (!args.empty()) {
      for (auto &unit : args) {
        base->emplace_back(FetchObject(unit));
      }
    }

    Object obj(base, kTypeIdArray);
    obj.SetConstructorFlag();
    frame.RefreshReturnStack(obj);
  }

  void Machine::CommandReturn(ArgumentList &args) {
    if (frame_stack_.size() <= 1) {
      trace::AddEvent("Unexpected return.", kStateError);
      return;
    }

    if (args.size() == 1) {
      Object ret_obj = FetchObject(args[0]);

      auto *container = &obj_stack_.GetCurrent();
      while (container->Find(kStrUserFunc) == nullptr) {
        obj_stack_.Pop();
        container = &obj_stack_.GetCurrent();
      }

      RecoverLastState();
      frame_stack_.top().RefreshReturnStack(ret_obj);
    }
    else if (args.size() == 0) {
      auto *container = &obj_stack_.GetCurrent();
      while (container->Find(kStrUserFunc) == nullptr) {
        obj_stack_.Pop();
        container = &obj_stack_.GetCurrent();
      }

      RecoverLastState();
      frame_stack_.top().RefreshReturnStack(Object());
    }
    else {
      ManagedArray obj_array = make_shared<ObjectArray>();
      for (auto it = args.begin(); it != args.end(); ++it) {
        obj_array->emplace_back(FetchObject(*it));
      }
      Object ret_obj(obj_array, kTypeIdArray);

      auto *container = &obj_stack_.GetCurrent();
      while (container->Find(kStrUserFunc) == nullptr) {
        obj_stack_.Pop();
        container = &obj_stack_.GetCurrent();
      }

      RecoverLastState();
      frame_stack_.top().RefreshReturnStack(ret_obj);
    }
  }

  void Machine::MachineCommands(Keyword token, ArgumentList &args, Request &request) {
    auto &frame = frame_stack_.top();

    switch (token) {
    case kKeywordPlus:
      BinaryMathOperatorImpl<kKeywordPlus>(args);
      break;
    case kKeywordMinus:
      BinaryMathOperatorImpl<kKeywordMinus>(args);
      break;
    case kKeywordTimes:
      BinaryMathOperatorImpl<kKeywordTimes>(args);
      break;
    case kKeywordDivide:
      BinaryMathOperatorImpl<kKeywordDivide>(args);
      break;
    case kKeywordEquals:
      BinaryLogicOperatorImpl<kKeywordEquals>(args);
      break;
    case kKeywordLessOrEqual:
      BinaryLogicOperatorImpl<kKeywordLessOrEqual>(args);
      break;
    case kKeywordGreaterOrEqual:
      BinaryLogicOperatorImpl<kKeywordGreaterOrEqual>(args);
      break;
    case kKeywordNotEqual:
      BinaryLogicOperatorImpl<kKeywordNotEqual>(args);
      break;
    case kKeywordGreater:
      BinaryLogicOperatorImpl<kKeywordGreater>(args);
      break;
    case kKeywordLess:
      BinaryLogicOperatorImpl<kKeywordLess>(args);
      break;
    case kKeywordAnd:
      BinaryLogicOperatorImpl<kKeywordAnd>(args);
      break;
    case kKeywordOr:
      BinaryLogicOperatorImpl<kKeywordOr>(args);
      break;
    case kKeywordNot:
      OperatorLogicNot(args);
      break;
    case kKeywordHash:
      CommandHash(args);
      break;
    case kKeywordFor:
      CommandForEach(args, request.option.nest_end);
      break;
    case kKeywordNullObj:
      CommandNullObj(args);
      break;
    case kKeywordDestroy:
      CommandDestroy(args);
      break;
    case kKeywordConvert:
      CommandConvert(args);
      break;
    case kKeywordRefCount:
      CommandRefCount(args);
      break;
    case kKeywordTime:
      CommandTime();
      break;
    case kKeywordVersion:
      CommandVersion();
      break;
    case kKeywordCodeName:
      CommandMachineCodeName();
      break;
    case kKeywordSwap:
      CommandSwap(args);
      break;
    case kKeywordBind:
      CommandBind(args, request.option.local_object);
      break;
    case kKeywordExpList:
      ExpList(args);
      break;
    case kKeywordInitialArray:
      InitArray(args);
      break;
    case kKeywordReturn:
      CommandReturn(args);
      break;
    case kKeywordTypeId:
      CommandTypeId(args);
      break;
    case kKeywordDir:
      CommandMethods(args);
      break;
    case kKeywordExist:
      CommandExist(args);
      break;
    case kKeywordFn:
      ClosureCatching(args, request.option.nest_end, frame_stack_.size() > 1);
      break;
    case kKeywordCase:
      CommandCase(args, request.option.nest_end);
      break;
    case kKeywordWhen:
      CommandWhen(args);
      break;
    case kKeywordEnd:
      switch (request.option.nest_root) {
      case kKeywordWhile:
        CommandLoopEnd(request.option.nest);
        break;
      case kKeywordFor:
        CommandForEachEnd(request.option.nest);
        break;
      case kKeywordIf:
      case kKeywordCase:
        CommandConditionEnd();
        break;
      default:break;
      }

      break;
    case kKeywordContinue:
    case kKeywordBreak:
      CommandContinueOrBreak(token, request.option.escape_depth);
      break;
    case kKeywordElse:
      CommandElse();
      break;
    case kKeywordIf:
    case kKeywordElif:
    case kKeywordWhile:
      CommandIfOrWhile(token, args, request.option.nest_end);
      break;
    default:
      break;
    }
  }

  void Machine::GenerateArgs(Interface &interface, ArgumentList &args, ObjectMap &obj_map) {
    switch (interface.GetArgumentMode()) {
    case kCodeNormalParam:
      Generate_Normal(interface, args, obj_map);
      break;
    case kCodeAutoSize:
      Generate_AutoSize(interface, args, obj_map);
      break;
    case kCodeAutoFill:
      Generate_AutoFill(interface, args, obj_map);
      break;
    default:
      break;
    }
  }

  void Machine::Generate_Normal(Interface &interface, ArgumentList &args, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    auto &params = interface.GetParameters();
    size_t pos = args.size() - 1;

    ERROR_CHECKING(args.size() > params.size(), 
      "Too many arguments");
    ERROR_CHECKING(args.size() < params.size(), 
      "Youe need at least " + to_string(params.size()) + "argument(s).");


    for (auto it = params.rbegin(); it != params.rend(); ++it) {
      obj_map.emplace(NamedObject(*it, FetchObject(args[pos])));
      pos -= 1;
    }
  }

  void Machine::Generate_AutoSize(Interface &interface, ArgumentList &args, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    vector<string> &params = interface.GetParameters();
    list<Object> temp_list;
    ManagedArray va_base = make_shared<ObjectArray>();
    size_t pos = args.size(), diff = args.size() - params.size() + 1;

    ERROR_CHECKING(args.size() < params.size(),
      "Youe need at least " + to_string(params.size()) + "argument(s).");

    while (diff != 0) {
      temp_list.emplace_front(FetchObject(args[pos - 1]));
      pos -= 1;
      diff -= 1;
    }

    if (!temp_list.empty()) {
      for (auto it = temp_list.begin(); it != temp_list.end(); ++it) {
        va_base->emplace_back(*it);
      }

      temp_list.clear();
    }

    obj_map.insert(NamedObject(params.back(), Object(va_base, kTypeIdArray)));

    if (pos != 0) {
      while (pos > 0) {
        obj_map.emplace(params[pos - 1], FetchObject(args[pos - 1]));
        pos -= 1;
      }
    }
  }

  void Machine::Generate_AutoFill(Interface &interface, ArgumentList &args, ObjectMap &obj_map) {
    auto &frame = frame_stack_.top();
    auto &params = interface.GetParameters();
    size_t min_size = interface.GetMinArgSize();
    size_t pos = args.size() - 1, param_pos = params.size() - 1;

    ERROR_CHECKING(args.size() > params.size(),
      "Too many arguments");
    ERROR_CHECKING(args.size() < min_size, 
      "You need at least " + to_string(min_size) + "argument(s)");

    for (auto it = params.crbegin(); it != params.crend(); ++it) {
      if (param_pos != pos) {
        obj_map.emplace(NamedObject(*it, Object()));
      }
      else {
        obj_map.emplace(NamedObject(*it, FetchObject(args[pos])));
        pos -= 1;
      }
      param_pos -= 1;
    }
  }

  /*
    Main loop and invoking loop of virtual machine.
    This function contains main logic implementation.
    VM runs single command in every single tick of machine loop.
  */
  void Machine::Run(bool invoking, string id, VMCodePointer ptr, ObjectMap *p,
    ObjectMap *closure_record) {
    if (code_stack_.empty()) return;

    if (invoking) {
      code_stack_.push_back(ptr);
    }

    bool interface_error = false;
    bool invoking_error = false;
    size_t stop_point = invoking ? frame_stack_.size() : 0;
    size_t script_idx = 0;
    Message msg;
    VMCode *code = code_stack_.back();
    Command *command = nullptr;
    InterfacePointer interface;
    ObjectMap obj_map;

    frame_stack_.push(RuntimeFrame());
    obj_stack_.Push();

    if (invoking) {
      obj_stack_.CreateObject(kStrUserFunc, Object(id));
      obj_stack_.MergeMap(*p);
      obj_stack_.MergeMap(*closure_record);
    }

    RuntimeFrame *frame = &frame_stack_.top();
    size_t size = code->size();

    //Refreshing loop tick state to make it work correctly.
    auto refresh_tick = [&]() ->void {
      code = code_stack_.back();
      size = code->size();
      frame = &frame_stack_.top();
    };

    auto update_stack_frame = [&](Interface &func)->void {
      code_stack_.push_back(&func.GetCode());
      frame_stack_.push(RuntimeFrame());
      obj_stack_.Push();
      obj_stack_.CreateObject(kStrUserFunc, Object(func.GetId()));
      obj_stack_.MergeMap(obj_map);
      obj_stack_.MergeMap(interface->GetClosureRecord());
      refresh_tick();
      frame->jump_offset = func.GetOffset();
    };

    // Main loop of virtual machine.
    while (frame->idx < size || frame_stack_.size() > 1) {
      //stop at invoking point
      if (invoking && frame_stack_.size() == stop_point) {
        break;
      }

      //switch back to last stack frame
      if (frame->idx == size && frame_stack_.size() > 1) {
        RecoverLastState();
        refresh_tick();
        frame->RefreshReturnStack(Object());
        frame->Steping();
        continue;
      }


      if (command = &(*code)[frame->idx]; command->first.type == kRequestNull) {
        trace::AddEvent("Frontend Panic.", kStateError);
        break;
      }

      script_idx = command->first.idx;
      frame->void_call = command->first.option.void_call;

      //Embedded machine commands.
      if (command->first.type == kRequestCommand) {
        MachineCommands(command->first.GetKeywordValue(), command->second, command->first);
        
        if (command->first.GetKeywordValue() == kKeywordReturn) {
          refresh_tick();
        }

        if (frame->error) {
          script_idx = command->first.idx;
          break;
        }

        frame->Steping();
        continue;
      }

      //Querying function(Interpreter built-in or user-defined)
      if (command->first.type == kRequestInterface) {
        if (!FetchInterface(interface, command, obj_map)) {
          break;
        }
      }

      //Building object map for function call expressed by command
      obj_map.clear();
      
      if (GenerateArgs(*interface, command->second, obj_map); frame->error) {
        script_idx = command->first.idx;
        break;
      }

      //(For user-defined function)
      //Machine will create new stack frame and push IR pointer to machine stack,
      //and start new processing in next tick.
      if (interface->GetPolicyType() == kInterfaceVMCode) {
        update_stack_frame(*interface);
        continue;
      }
      else {
        msg = interface->Start(obj_map);
      }

      if (msg.GetLevel() == kStateError) {
        interface_error = true;
        break;
      }

      //Invoking by return value.
      if (msg.GetCode() == kCodeInterface) {
        if (auto arg = BuildStringVector(msg.GetDetail()); 
          !_FetchInterface(interface, arg[0], arg[1])) {
          break;
        }

        if (interface->GetPolicyType() == kInterfaceVMCode) {
          update_stack_frame(*interface);
        }
        else {
          msg = interface->Start(obj_map);
          frame->Steping();
        }
        continue;
      }

      frame->RefreshReturnStack(msg.GetObj());
      frame->Steping();
    }

    if (frame->error) {
      trace::AddEvent(
        Message(kCodeBadExpression, frame->error_string, kStateError)
          .SetIndex(script_idx));
      if (invoking) invoking_error = true;
    }

    if (interface_error) {
      trace::AddEvent(msg.SetIndex(script_idx));
      if (invoking) invoking_error = true;
    }

    if (!invoking || (invoking && frame_stack_.size() != stop_point)) {
      obj_stack_.Pop();
      frame_stack_.pop();
      code_stack_.pop_back();
    }

    if (invoking && invoking_error) {
      frame_stack_.top().MakeError("Invoking error is happend.");
    }
  }
}
