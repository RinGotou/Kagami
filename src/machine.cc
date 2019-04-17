#include "machine.h"

#define ERROR_CHECKING(_Cond, _Msg) if (_Cond) { worker.MakeError(_Msg); return; }

namespace kagami {
  using namespace management;

  PlainType FindTypeCode(string type_id) {
    auto it = kTypeStore.find(type_id);
    return it != kTypeStore.end() ? it->second : kNotPlainType;
  }

  int64_t IntProducer(Object &obj) {
    auto type = FindTypeCode(obj.GetTypeId());
    int64_t result = 0;
    switch (type) {
    case kPlainInt:result = obj.Cast<int64_t>(); break;
    case kPlainFloat:result = static_cast<int64_t>(obj.Cast<double>()); break;
    case kPlainBool:result = obj.Cast<bool>() ? 1 : 0; break;
    default:break;
    }

    return result;
  }

  double FloatProducer(Object &obj) {
    auto type = FindTypeCode(obj.GetTypeId());
    double result = 0;
    switch (type) {
    case kPlainFloat:result = obj.Cast<double>(); break;
    case kPlainInt:result = static_cast<double>(obj.Cast<int64_t>()); break;
    case kPlainBool:result = obj.Cast<bool>() ? 1.0 : 0.0; break;
    default:break;
    }

    return result;
  }

  string StringProducer(Object &obj) {
    auto type = FindTypeCode(obj.GetTypeId());
    string result;
    switch (type) {
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
#if defined(_WIN32) && defined(_MSC_VER)
  //from MSDN
  std::wstring s2ws(const std::string &s) {
    auto slength = static_cast<int>(s.length()) + 1;
    auto len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    auto *buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
  }

  std::string ws2s(const std::wstring &s) {
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
    return r;
  }
#else
  //from https://www.yasuhisay.info/interface/20090722/1248245439
  std::wstring s2ws(const std::string &s) {
    if (s.empty()) return wstring();
    size_t length = s.size();
    wchar_t *wc = (wchar_t *)malloc(sizeof(wchar_t) * (length + 2));
    mbstowcs(wc, s.c_str(), s.length() + 1);
    std::wstring str(wc);
    free(wc);
    return str;
  }

  std::string ws2s(const std::wstring & s) {
    if (s.empty()) return string();
    size_t length = s.size();
    char *c = (char *)malloc(sizeof(char) * length * 2);
    wcstombs(c, s.c_str(), s.length() + 1);
    std::string result(c);
    free(c);
    return result;
  }
#endif

  string ParseRawString(const string & src) {
    string result = src;
    if (util::IsString(result)) result = util::GetRawString(result);
    return result;
  }

  void InitPlainTypes() {
    using type::NewTypeSetup;
    using type::PlainHasher;

    NewTypeSetup(kTypeIdInt, SimpleSharedPtrCopy<int64_t>, PlainHasher<int64_t>());
    NewTypeSetup(kTypeIdFloat, SimpleSharedPtrCopy<double>, PlainHasher<double>());
    NewTypeSetup(kTypeIdBool, SimpleSharedPtrCopy<bool>, PlainHasher<bool>());
    NewTypeSetup(kTypeIdNull, FakeCopy<void>);

    EXPORT_CONSTANT(kTypeIdInt);
    EXPORT_CONSTANT(kTypeIdFloat);
    EXPORT_CONSTANT(kTypeIdBool);
    EXPORT_CONSTANT(kTypeIdNull);
  }

  void MachineWorker::MakeError(string str) {
    error = true;
    error_string = str;
  }

  void MachineWorker::SwitchToMode(MachineMode mode) {
    mode_stack.push(this->mode);
    this->mode = mode;
  }

  void MachineWorker::RefreshReturnStack(Object obj) {
    if (!void_call) {
      return_stack.push(std::move(obj));
    }
  }

  void MachineWorker::GoLastMode() {
    if (!mode_stack.empty()) {
      this->mode = mode_stack.top();
      mode_stack.pop();
    }
    else {
      MakeError("Mode switching error.VM Panic.");
    }
  }

  bool MachineWorker::NeedSkipping() {
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

  IRLoader::IRLoader(const char *src) : health(true) {
    bool comment_block = false;
    size_t error_counter = 0;
    size_t index_counter = 1;
    wstring buf;
    string temp;
    wifstream stream(src);
    Analyzer analyzer;
    Message msg;
    list<CombinedCodeline> script_buf;
    KIR ir;

    if (!stream.good()) {
      health = false;
      return;
    }

    while (!stream.eof()) {
      std::getline(stream, buf);
      temp = ws2s(buf);
      if (!temp.empty() && temp.back() == '\0') temp.pop_back();
      temp = IndentationAndCommentProc(temp);

      if (temp == kStrCommentBegin) {
        comment_block = true;
        index_counter += 1;
        continue;
      }

      if (comment_block) {
        if (temp == kStrCommentEnd) {
          comment_block = false;
        }

        index_counter += 1;
        continue;
      }

      if (temp.empty()) { 
        index_counter += 1;
        continue; 
      }
      script_buf.push_back(CombinedCodeline(index_counter, temp));
      index_counter += 1;
    }

    for (auto it = script_buf.begin(); it != script_buf.end(); ++it) {
      if (!health) {
        if (error_counter < MAX_ERROR_COUNT) {
          error_counter += 1;
        }
        else {
          trace::AddEvent(kCodeIllegalSymbol, 
            "Too many errors. Analyzing is stopped.");
          break;
        }
      }

      msg = analyzer.Make(it->second, it->first).SetIndex(it->first);

      switch (msg.GetLevel()) {
      case kStateError:
        trace::AddEvent(msg);
        health = false;
        continue;
        break;
      case kStateWarning:
        trace::AddEvent(msg);
        break;
      default:break;
      }

      ir = analyzer.GetOutput();
      analyzer.Clear();
      output.insert(output.end(), ir.begin(), ir.end());
      ir.clear();
    }
  }

  /* Disposing all indentation and comments */
  string IRLoader::IndentationAndCommentProc(string target) {
    if (target == "") return "";
    string data;
    char current = 0, last = 0;
    size_t head = 0, tail = 0;
    bool exempt_blank_char = true;
    bool string_processing = false;
    auto toString = [](char t) ->string {return string().append(1, t); };

    for (size_t count = 0; count < target.size(); ++count) {
      current = target[count];
      auto type = util::GetTokenType(toString(current));
      if (type != TokenType::kTokenTypeBlank && exempt_blank_char) {
        head = count;
        exempt_blank_char = false;
      }
      if (current == '\'' && last != '\\') {
        string_processing = !string_processing;
      }
      if (!string_processing && current == '#') {
        tail = count;
        break;
      }
      last = target[count];
    }
    if (tail > head) data = target.substr(head, tail - head);
    else data = target.substr(head, target.size() - head);
    if (data.front() == '#') return "";

    while (!data.empty() &&
      util::GetTokenType(toString(data.back())) == kTokenTypeBlank) {
      data.pop_back();
    }
    return data;
  }

  void Machine::RecoverLastState() {
    worker_stack_.pop();
    ir_stack_.pop_back();
    obj_stack_.Pop();
  }

  Object Machine::FetchPlainObject(Argument &arg) {
    auto type = arg.token_type;
    auto &value = arg.data;
    Object obj;
    switch (type) {
    case kTokenTypeInt:
      obj.Manage(make_shared<int64_t>(stol(value)), kTypeIdInt); 
      break;
    case kTokenTypeFloat:
      obj.Manage(make_shared<double>(stod(value)), kTypeIdFloat);
      break;
    case kTokenTypeBool:
      obj.Manage(make_shared<bool>(value == kStrTrue), kTypeIdBool);
      break;
    case kTokenTypeString:
      obj.Manage(make_shared<string>(ParseRawString(value)), kTypeIdString);
      break;
    case kTokenTypeGeneric:
      obj.Manage(make_shared<string>(value), kTypeIdString);
      break;
    default:
      break;
    }

    return obj;
  }

  Object Machine::FetchInterfaceObject(string id, string domain) {
    Object obj;
    auto &worker = worker_stack_.top();
    auto ptr = FindInterface(id, domain);

    if (ptr != nullptr) {
      auto interface = *FindInterface(id, domain);
      obj.Manage(make_shared<Interface>(interface), kTypeIdFunction);
    }

    return obj;
  }

  string Machine::FetchDomain(string id, ArgumentType type) {
    auto &worker = worker_stack_.top();
    auto &return_stack = worker.return_stack;
    ObjectPointer ptr = nullptr;
    string result;

    if (type == kArgumentObjectStack) {
      ptr = obj_stack_.Find(id);
      if (ptr != nullptr) {
        result = ptr->GetTypeId();
        return result;
      }

      Object obj = GetConstantObject(id);

      if (obj.Get() != nullptr) {
        result = obj.GetTypeId();
        return result;
      }
      
      //TODO:??
      obj = FetchInterfaceObject(id, kTypeIdNull);
      if (obj.Get() != nullptr) {
        result = obj.GetTypeId();
        return result;
      }
    }
    else if (type == kArgumentReturnStack) {
      if (!return_stack.empty()) {
        result = return_stack.top().GetTypeId();
      }
    }

    if (result.empty()) {
      worker.MakeError("Domain is not found - " + id);
    }

    return result;
  }

  Object Machine::FetchObject(Argument &arg, bool checking) {
    if (arg.type == kArgumentNormal) {
      return FetchPlainObject(arg);
    }

    auto &worker = worker_stack_.top();
    auto &return_stack = worker.return_stack;
    string domain_type_id = kTypeIdNull;
    ObjectPointer ptr = nullptr;
    Object obj;

    //TODO: Add object domain support
    if (arg.domain.type != kArgumentNull) {
      domain_type_id = FetchDomain(arg.domain.data, arg.domain.type);
    }

    if (arg.type == kArgumentObjectStack) {
      ptr = obj_stack_.Find(arg.data);
      if (ptr != nullptr) {
        obj.CreateRef(*ptr);
        return obj;
      }

      obj = GetConstantObject(arg.data);

      if (obj.Get() == nullptr) {
        obj = FetchInterfaceObject(arg.data, domain_type_id);
      }

      if (obj.Get() == nullptr) {
        worker.MakeError("Object is not found - " + arg.data);
      }
    }
    else if (arg.type == kArgumentReturnStack) {
      if (!return_stack.empty()) {
        obj = return_stack.top();
        if(!checking) return_stack.pop(); //For DomainAssertR
      }
      else {
        worker.MakeError("Can't get object from stack.");
      }
    }

    return obj;
  }

  bool Machine::_FetchInterface(InterfacePointer &interface, string id, string type_id) {
    auto &worker = worker_stack_.top();

    //Modified version for function invoking
    if (type_id != kTypeIdNull) {
      if (!type::CheckMethod(id, type_id)) {
        worker.MakeError("Method is not found - " + id);
        return false;
      }

      interface = FindInterface(id, type_id);
      return true;
    }
    else {
      interface = FindInterface(id);

      if (interface != nullptr) return true;

      ObjectPointer ptr = obj_stack_.Find(id);

      if (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction) {
        interface = &ptr->Cast<Interface>();
        return true;
      }

      worker.MakeError("Function is not found - " + id);
    }

    return false;
  }

  bool Machine::FetchInterface(InterfacePointer &interface, CommandPointer &command, ObjectMap &obj_map) {
    auto &id = command->first.head_interface;
    auto &domain = command->first.domain;
    auto &worker = worker_stack_.top();

    //Object methods.
    //In current developing processing, machine forced to querying built-in
    //function. These code need to be rewritten when I work in class feature in
    //the future.
    if (command->first.domain.type != kArgumentNull) {
      Object obj = FetchObject(domain, true);

      if (worker.error) return false;

      if (!type::CheckMethod(id, obj.GetTypeId())) {
        worker.MakeError("Method is not found - " + id);
        return false;
      }

      interface = FindInterface(id, obj.GetTypeId());
      obj_map.emplace(NamedObject(kStrMe, obj));
      return true;
    }
    //Plain bulit-in function and user-defined function
    //At first, Machine will querying in built-in function map,
    //and then try to fetch function object in heap.
    else {
      interface = FindInterface(id);

      if (interface != nullptr) return true;

      ObjectPointer ptr = obj_stack_.Find(id);

      if (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction) {
        interface = &ptr->Cast<Interface>();
        return true;
      }

      worker.MakeError("Function is not found - " + id);
    }

    return false;
  }

  void Machine::InitFunctionCatching(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    worker.fn_string_vec.clear();
    if (!args.empty()) {
      for (auto it = args.begin(); it != args.end(); ++it) {
        worker.fn_string_vec.emplace_back(it->data);
      }
    }
    else {
      worker.MakeError("Empty argument list.");
    }

    worker.fn_idx = worker.idx;
    worker.SwitchToMode(kModeClosureCatching);
  }

  void Machine::FinishFunctionCatching(bool closure) {
    auto &obj_list = obj_stack_.GetBase();
    auto &worker = worker_stack_.top();
    auto &origin_ir = *ir_stack_.back();
    auto &fn_string_vec = worker.fn_string_vec;
    bool optional = false;
    bool variable = false;
    StateCode argument_mode = kCodeNormalParam;
    size_t counter = 0;
    size_t size = worker.fn_string_vec.size();
    vector<string> params;
    KIR ir;

    for (size_t idx = worker.fn_idx + 1; idx < worker.idx - 1; idx += 1) {
      ir.emplace_back(origin_ir[idx]);
    }

    for (size_t idx = 1; idx < size; idx += 1) {
      if (fn_string_vec[idx] == kStrOptional) {
        optional = true;
        counter += 1;
        continue;
      }

      if (fn_string_vec[idx] == kStrVariable) {
        if (counter == 1) {
          worker.MakeError("Variable parameter can be defined only once.");
          break;
        }

        if (idx != size - 2) {
          worker.MakeError("Variable parameter must be last one.");
          break;
        }

        variable = true;
        counter += 1;
        continue;
      }

      if (optional && fn_string_vec[idx - 1] != kStrOptional) {
        worker.MakeError("Optional parameter must be defined after normal parameters.");
      }

      params.push_back(fn_string_vec[idx]);
    }

    if (optional && variable) {
      worker.MakeError("Variable and optional parameter can't be defined at same time.");
      return;
    }

    if (optional) argument_mode = kCodeAutoFill;
    if (variable) argument_mode = kCodeAutoSize;

    Interface interface(ir, fn_string_vec[0], params, argument_mode);

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

    obj_stack_.CreateObject(fn_string_vec[0],
      Object(make_shared<Interface>(interface), kTypeIdFunction));


    worker.GoLastMode();
  }

  void Machine::Skipping(bool enable_terminators, 
    initializer_list<GenericToken> terminators) {
    auto &worker = worker_stack_.top();
    size_t nest_counter = 0;
    size_t size = ir_stack_.back()->size();
    bool flag = false;
    auto &ir = *ir_stack_.back();

    if (ir[worker.idx].first.head_command == kTokenEnd && worker.skipping_count == 0) 
      return;

    while (worker.idx < size) {
      Command &command = ir[worker.idx];

      if (command.first.head_command != kTokenSegment) {
        worker.idx += 1;
        continue;
      }

      SetSegmentInfo(command.second, true);

      if (find_in_vector(worker.last_command, nest_flag_collection)) {
        nest_counter += 1;
        worker.idx += 1;
        continue;
      }

      if (enable_terminators && compare(worker.last_command, terminators)) {
        if (nest_counter == 0) {
          flag = true;
          break;
        }
          
        worker.idx += 1;
        continue;
      }

      if (worker.last_command == kTokenEnd) {
        if (nest_counter != 0) {
          nest_counter -= 1;
          worker.idx += 1;
          continue;
        }

        if (worker.skipping_count > 0) {
          worker.skipping_count -= 1;
          worker.idx += 1;
          continue;
        }

        flag = true;
        break;
      }

      worker.idx += 1;
    }

    if (!flag) {
      worker.MakeError("Expect 'end'");
    }
  }

  //TODO:new user-defined function support
  Message Machine::Invoke(Object obj, string id, const initializer_list<NamedObject> &&args) {
    auto &worker = worker_stack_.top();

    bool found = type::CheckMethod(id, obj.GetTypeId());
    InterfacePointer interface;

    if (!found) {
      //Immediately push event to avoid ugly checking block.
      trace::AddEvent("Method is not found" + id);
      return Message();
    }

    _FetchInterface(interface, id, obj.GetTypeId());


    ObjectMap obj_map = args;
    obj_map.insert(NamedObject(kStrMe, obj));

    return interface->Start(obj_map);
  }

  void Machine::SetSegmentInfo(ArgumentList &args, bool cmd_info) {
    auto &worker = worker_stack_.top();
    worker.logic_idx = worker.idx;
    if (cmd_info) {
      worker.last_command = static_cast<GenericToken>(stol(args[0].data));
    }
  }

  void Machine::CommandHash(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    auto &obj = FetchObject(args[0]).Deref();

    if (type::IsHashable(obj)) {
      int64_t hash = type::GetHash(obj);
      worker.RefreshReturnStack(Object(make_shared<int64_t>(hash), kTypeIdInt));
    }
    else {
      worker.RefreshReturnStack(Object());
    }
  }

  void Machine::CommandSwap(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    auto &right = FetchObject(args[1]).Deref();
    auto &left = FetchObject(args[0]).Deref();

    left.swap(right);
  }

  void Machine::CommandIfOrWhile(GenericToken token, ArgumentList &args) {
    auto &worker = worker_stack_.top();
    REQUIRED_ARG_COUNT(1);

    Object obj = FetchObject(args[0]);
    ERROR_CHECKING(obj.GetTypeId() != kTypeIdBool, "Invalid state value type.");

    bool state = obj.Cast<bool>();

    if (token == kTokenIf) {
      worker.SwitchToMode(state ? kModeCondition : kModeNextCondition);
      obj_stack_.Push();
      worker.condition_stack.push(state);
    }
    else if (token == kTokenWhile) {
      if (worker.loop_head.empty()) {
        obj_stack_.Push();
        worker.mode_stack.push(worker.mode);
      }
      else if (worker.loop_head.top() != worker.logic_idx - 1) {
        obj_stack_.Push();
        worker.mode_stack.push(worker.mode);
      }

      if (worker.loop_head.empty() || worker.loop_head.top() != worker.logic_idx - 1) {
        worker.loop_head.push(worker.logic_idx - 1);
      }

      if (state) {
        worker.mode = kModeCycle;
      }
      else {
        worker.mode = kModeCycleJump;
        if (worker.loop_head.size() == worker.loop_tail.size()) {
          if (!worker.loop_tail.empty()) {
            worker.idx = worker.loop_tail.top();
          }
        }
      }
    }
    else if (token == kTokenElif) {
      ERROR_CHECKING(worker.condition_stack.empty(), "Unexpected Elif.");

      if (worker.condition_stack.top() == false && worker.mode == kModeNextCondition) {
        worker.mode = kModeCondition;
        worker.condition_stack.top() = true;
      }
    }
  }

  void Machine::CommandForEach(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    ObjectMap obj_map;

    if (!worker.loop_head.empty() && worker.loop_head.top() == worker.logic_idx - 1) {
      ForEachChecking(args);
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

    auto unit = Invoke(iterator_obj, "get").GetObj();

    obj_stack_.Push();
    obj_stack_.CreateObject(kStrIteratorObj, iterator_obj);
    obj_stack_.CreateObject(unit_id, unit);
    worker.loop_head.push(worker.logic_idx - 1);
    worker.SwitchToMode(kModeForEach);
  }

  void Machine::ForEachChecking(ArgumentList &args) {
    auto &worker = worker_stack_.top();
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
      worker.mode = kModeForEachJump;
    }
    else {
      auto unit = Invoke(iterator, "get").GetObj();
      obj_stack_.CreateObject(unit_id, unit);
    }
  }

  void Machine::CommandElse() {
    auto &worker = worker_stack_.top();
    ERROR_CHECKING(worker.condition_stack.empty(), "Unexpected Else.");

    if (worker.condition_stack.top() == true) {
      switch (worker.mode) {
      case kModeCondition:
      case kModeNextCondition:
        worker.mode = kModeNextCondition;
        break;
      case kModeCase:
      case kModeCaseJump:
        worker.mode = kModeCaseJump;
        break;
      default:
        break;
      }
    }
    else {
      worker.condition_stack.top() = true;
      switch (worker.mode) {
      case kModeNextCondition:
        worker.mode = kModeCondition;
        break;
      case kModeCaseJump:
        worker.mode = kModeCase;
        break;
      default:
        break;
      }
    }
  }

  void Machine::CommandCase(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    ERROR_CHECKING(args.empty(), "Empty argument list");

    Object obj = FetchObject(args[0]);
    string type_id = obj.GetTypeId();
    ERROR_CHECKING(!util::IsPlainType(type_id), 
      "Non-plain object is not supported by case");

    Object sample_obj = type::CreateObjectCopy(obj);
    obj_stack_.Push();
    obj_stack_.CreateObject(kStrCaseObj, sample_obj);
    worker.SwitchToMode(kModeCaseJump);
    worker.condition_stack.push(false);

  }

  void Machine::CommandWhen(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    bool result = false;
    ERROR_CHECKING(worker.condition_stack.empty(), 
      "Unexpected 'when'");
    ERROR_CHECKING(!compare(worker.mode, { kModeCase,kModeCaseJump }),
      "Unexpected 'when'");

    if (worker.mode == kModeCase) {
      worker.mode = kModeCaseJump;
      return;
    }
    
    if (worker.condition_stack.top()) {
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
        worker.mode = kModeCase;
        worker.condition_stack.top() = true;
      }
    }
  }

  void Machine::CommandContinueOrBreak(GenericToken token) {
    auto &worker = worker_stack_.top();
    while (!worker.mode_stack.empty()) {
      if (worker.mode == kModeCycle) break;
      if (worker.mode == kModeForEach) break;

      if (worker.mode == kModeCondition || worker.mode == kModeCase) {
        worker.condition_stack.pop();
        worker.skipping_count += 1;
      }

      worker.GoLastMode();
    }

    ERROR_CHECKING(worker.mode != kModeCycle && worker.mode != kModeForEach,
      "Invalid 'continue'/'break'");

    if (worker.mode == kModeCycle) worker.mode = kModeCycleJump;
    if (worker.mode == kModeForEach) worker.mode = kModeForEachJump;

    switch (token) {
    case kTokenContinue:worker.activated_continue = true; break;
    case kTokenBreak:worker.activated_break = true; break;
    default:break;
    }
  }

  void Machine::CommandConditionEnd() {
    auto &worker = worker_stack_.top();
    worker.condition_stack.pop();
    worker.GoLastMode();
    obj_stack_.Pop();
  }

  void Machine::CommandLoopEnd() {
    auto &worker = worker_stack_.top();
    if (worker.mode == kModeCycle) {
      if (worker.loop_tail.empty() || worker.loop_tail.top() != worker.logic_idx - 1) {
        worker.loop_tail.push(worker.logic_idx - 1);
      }
      worker.idx = worker.loop_head.top();
      while (!worker.return_stack.empty()) worker.return_stack.pop();
      obj_stack_.GetCurrent().clear();
    }
    else if (worker.mode == kModeCycleJump) {
      if (worker.activated_continue) {
        if (worker.loop_tail.empty() || worker.loop_tail.top() != worker.logic_idx - 1) {
          worker.loop_tail.push(worker.logic_idx - 1);
        }
        worker.idx = worker.loop_head.top();
        worker.mode = kModeCycle;
        worker.activated_continue = false;
        obj_stack_.GetCurrent().clear();
      }
      else {
        if (worker.activated_break) worker.activated_break = false;
        while (!worker.return_stack.empty()) worker.return_stack.pop();
        worker.GoLastMode();
        if (worker.loop_head.size() == worker.loop_tail.size()) {
          worker.loop_head.pop();
          worker.loop_tail.pop();
        }
        else {
          worker.loop_head.pop();
        }
        obj_stack_.Pop();
      }
    }
  }

  void Machine::CommandForEachEnd() {
    auto &worker = worker_stack_.top();
    if (worker.mode == kModeForEach) {
      if (worker.loop_tail.empty() || worker.loop_tail.top() != worker.logic_idx - 1) {
        worker.loop_tail.push(worker.logic_idx - 1);
      }
      worker.idx = worker.loop_head.top();
      obj_stack_.GetCurrent().ClearExcept(kStrIteratorObj);
    }
    else if (worker.mode == kModeForEachJump) {
      if (worker.activated_continue) {
        if (worker.loop_tail.empty() || worker.loop_tail.top() != worker.logic_idx - 1) {
          worker.loop_tail.push(worker.logic_idx - 1);
        }
        worker.idx = worker.loop_head.top();
        worker.mode = kModeForEach;
        worker.activated_continue = false;
        obj_stack_.GetCurrent().ClearExcept(kStrIteratorObj);
      }
      else {
        if (worker.activated_break) worker.activated_break = false;
        worker.GoLastMode();
        if (worker.loop_head.size() == worker.loop_tail.size()) {
          worker.loop_head.pop();
          worker.loop_tail.pop();
        }
        else {
          worker.loop_head.pop();
        }
        obj_stack_.Pop();
      }
    }
  }

  void Machine::CommandBind(ArgumentList &args) {
    using namespace type;
    auto &worker = worker_stack_.top();
    //Do not change the order!
    auto rhs = FetchObject(args[1]);
    auto lhs = FetchObject(args[0]);

    if (lhs.IsRef()) {
      auto &real_lhs = lhs.Deref();
      real_lhs = CreateObjectCopy(rhs);
      return;
    }
    else {
      string id = lhs.Cast<string>();
      ObjectPointer ptr = obj_stack_.Find(id);

      if (ptr != nullptr) {
        ptr->Deref() = CreateObjectCopy(rhs);
      }
      else {
        Object obj = CreateObjectCopy(rhs);
        ERROR_CHECKING(util::GetTokenType(id) != kTokenTypeGeneric,
          "Invalid object id.");
        ERROR_CHECKING(!obj_stack_.CreateObject(id, obj),
          "Object binding failed.");
      }
    }
  }

  void Machine::CommandTypeId(ArgumentList &args) {
    auto &worker = worker_stack_.top();

    if (args.size() > 1) {
      ManagedArray base = make_shared<ObjectArray>();

      for (auto &unit : args) {
        base->emplace_back(Object(FetchObject(unit).GetTypeId()));
      }

      Object obj(base, kTypeIdArray);
      obj.SetConstructorFlag();
      worker.RefreshReturnStack(obj);
    }
    else if (args.size() == 1) {
      worker.RefreshReturnStack(Object(FetchObject(args[0]).GetTypeId()));
    }
    else {
      worker.RefreshReturnStack(Object(kTypeIdNull));
    }
  }

  void Machine::CommandMethods(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    REQUIRED_ARG_COUNT(1);

    Object obj = FetchObject(args[0]);
    auto methods = type::GetMethods(obj.GetTypeId());
    ManagedArray base = make_shared<ObjectArray>();

    for (auto &unit : methods) {
      base->emplace_back(Object(unit, kTypeIdString));
    }

    Object ret_obj(base, kTypeIdArray);
    ret_obj.SetConstructorFlag();
    worker.RefreshReturnStack(ret_obj);
  }

  void Machine::CommandExist(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    REQUIRED_ARG_COUNT(2);

    //Do not change the order
    auto str_obj = FetchObject(args[1]);
    auto obj = FetchObject(args[0]);
    ERROR_CHECKING(str_obj.GetTypeId() != kTypeIdString, "Invalid method id");

    string str = str_obj.Cast<string>();
    Object ret_obj(type::CheckMethod(str, obj.GetTypeId()), kTypeIdBool);

    worker.RefreshReturnStack(ret_obj);
  }

  void Machine::CommandNullObj(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    REQUIRED_ARG_COUNT(1);

    Object obj = FetchObject(args[0]);
    worker.RefreshReturnStack(Object(obj.GetTypeId() == kTypeIdNull, kTypeIdBool));
  }

  void Machine::CommandDestroy(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    REQUIRED_ARG_COUNT(1);

    Object &obj = FetchObject(args[0]).Deref();
    obj.swap(Object());
  }

  void Machine::CommandConvert(ArgumentList &args) {
    auto &worker = worker_stack_.top();
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
        auto type = util::GetTokenType(str, true);

        switch (type) {
        case kTokenTypeInt:
          ret_obj.Manage(make_shared<int64_t>(stol(str)), kTypeIdInt);
          break;
        case kTokenTypeFloat:
          ret_obj.Manage(make_shared<double>(stod(str)), kTypeIdFloat);
          break;
        case kTokenTypeBool:
          ret_obj.Manage(make_shared<bool>(str == kStrTrue), kTypeIdBool);
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

      worker.RefreshReturnStack(ret_obj);
    }
  }

  void Machine::CommandRefCount(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    REQUIRED_ARG_COUNT(1);

    auto &obj = FetchObject(args[0]).Deref();
    Object ret_obj(make_shared<int64_t>(obj.ObjRefCount()), kTypeIdInt);

    worker.RefreshReturnStack(ret_obj);
  }

  void Machine::CommandTime() {
    auto &worker = worker_stack_.top();
    time_t now = time(nullptr);
    string nowtime(ctime(&now));
    nowtime.pop_back();
    worker.RefreshReturnStack(Object(nowtime));
  }

  void Machine::CommandVersion() {
    auto &worker = worker_stack_.top();
    worker.RefreshReturnStack(Object(kInterpreterVersion));
  }

  void Machine::CommandPatch() {
    auto &worker = worker_stack_.top();
    worker.RefreshReturnStack(Object(kPatchName));
  }

  template <GenericToken op_code>
  void Machine::BinaryMathOperatorImpl(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    REQUIRED_ARG_COUNT(2);
    auto rhs = FetchObject(args[1]);
    auto lhs = FetchObject(args[0]);
    auto type_rhs = FindTypeCode(rhs.GetTypeId());
    auto type_lhs = FindTypeCode(lhs.GetTypeId());

    if (type_rhs == kNotPlainType || type_rhs == kNotPlainType) {
      worker.RefreshReturnStack();
      return;
    }

    auto result_type = kResultDynamicTraits.at(ResultTraitKey(type_lhs, type_rhs));

#define RESULT_PROCESSING(_Type, _Func, _TypeId)                       \
  _Type result = MathBox<_Type, op_code>().Do(_Func(lhs), _Func(rhs)); \
  worker.RefreshReturnStack(Object(result, _TypeId));

    if (result_type == kPlainString) {
      if (!find_in_vector(op_code, kStringOpStore)) {
        worker.RefreshReturnStack();
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

  template <GenericToken op_code>
  void Machine::BinaryLogicOperatorImpl(ArgumentList &args) {
    using namespace type;
    auto &worker = worker_stack_.top();

    REQUIRED_ARG_COUNT(2);

    auto rhs = FetchObject(args[1]);
    auto lhs = FetchObject(args[0]);
    auto type_rhs = FindTypeCode(rhs.GetTypeId());
    auto type_lhs = FindTypeCode(lhs.GetTypeId());
    bool result = false;

    if (!util::IsPlainType(lhs.GetTypeId())) {
      if (op_code != kTokenEquals && op_code != kTokenNotEqual) {
        worker.RefreshReturnStack();
        return;
      }

      if (!CheckMethod(kStrCompare, lhs.GetTypeId())) {
        worker.MakeError("Can't operate with this operator.");
        return;
      }

      Object obj = Invoke(lhs, kStrCompare,
        { NamedObject(kStrRightHandSide, rhs) }).GetObj();

      if (obj.GetTypeId() != kTypeIdBool) {
        worker.MakeError("Invalid behavior of compare().");
        return;
      }

      if (op_code == kTokenNotEqual) {
        bool value = !obj.Cast<bool>();
        worker.RefreshReturnStack(Object(value, kTypeIdBool));
      }
      else {
        worker.RefreshReturnStack(obj);
      }
      return;
    }

    auto result_type = kResultDynamicTraits.at(ResultTraitKey(type_lhs, type_rhs));
#define RESULT_PROCESSING(_Type, _Func)\
  result = LogicBox<_Type, op_code>().Do(_Func(lhs), _Func(rhs));

    if (result_type == kPlainString) {
      if (!find_in_vector(op_code, kStringOpStore)) {
        worker.RefreshReturnStack();
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

    worker.RefreshReturnStack(Object(result, kTypeIdBool));
#undef RESULT_PROCESSING
  }

  void Machine::OperatorLogicNot(ArgumentList &args) {
    auto &worker = worker_stack_.top();

    REQUIRED_ARG_COUNT(1);

    auto rhs = FetchObject(args[0]);

    if (rhs.GetTypeId() != kTypeIdBool) {
      worker.MakeError("Can't operate with this operator");
      return;
    }

    bool result = !rhs.Cast<bool>();

    worker.RefreshReturnStack(Object(result, kTypeIdBool));
  }


  void Machine::ExpList(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    if (!args.empty()) {
      worker.RefreshReturnStack(FetchObject(args.back()));
    }
  }

  void Machine::InitArray(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    ManagedArray base = make_shared<ObjectArray>();

    if (!args.empty()) {
      for (auto &unit : args) {
        base->emplace_back(FetchObject(unit));
      }
    }

    Object obj(base, kTypeIdArray);
    obj.SetConstructorFlag();
    worker.RefreshReturnStack(obj);
  }

  void Machine::DomainAssert(ArgumentList &args) {
    auto &worker = worker_stack_.top();
    Object obj = FetchObject(args[0], true);
    string id = FetchObject(args[1]).Cast<string>();

    auto interface = FindInterface(id, obj.GetTypeId());
    ERROR_CHECKING(interface = nullptr, "Method is not found - " + id);

    Object ret_obj(*interface, kTypeIdFunction);
    worker.RefreshReturnStack(ret_obj);
  }

  void Machine::CommandReturn(ArgumentList &args) {
    if (worker_stack_.size() <= 1) {
      trace::AddEvent(kCodeBadExpression, "Unexpected return.", kStateError);
      return;
    }

    auto *container = &obj_stack_.GetCurrent();
    while (container->Find(kStrUserFunc) == nullptr) {
      obj_stack_.Pop();
      container = &obj_stack_.GetCurrent();
    }

    if (args.size() == 1) {
      Object src_obj = FetchObject(args[0]);
      Object ret_obj = type::CreateObjectCopy(src_obj);
      RecoverLastState();
      worker_stack_.top().RefreshReturnStack(ret_obj);
    }
    else if (args.size() == 0) {
      RecoverLastState();
      worker_stack_.top().RefreshReturnStack(Object());
    }
    else {
      ManagedArray obj_array = make_shared<ObjectArray>();
      for (auto it = args.begin(); it != args.end(); ++it) {
        obj_array->emplace_back(FetchObject(*it));
      }
      Object ret_obj(obj_array, kTypeIdArray);
      RecoverLastState();
      worker_stack_.top().RefreshReturnStack(ret_obj);
    }
  }

  void Machine::MachineCommands(GenericToken token, ArgumentList &args, Request &request) {
    auto &worker = worker_stack_.top();

    switch (token) {
    case kTokenPlus:
      BinaryMathOperatorImpl<kTokenPlus>(args);
      break;
    case kTokenMinus:
      BinaryMathOperatorImpl<kTokenMinus>(args);
      break;
    case kTokenTimes:
      BinaryMathOperatorImpl<kTokenTimes>(args);
      break;
    case kTokenDivide:
      BinaryMathOperatorImpl<kTokenDivide>(args);
      break;
    case kTokenEquals:
      BinaryLogicOperatorImpl<kTokenEquals>(args);
      break;
    case kTokenLessOrEqual:
      BinaryLogicOperatorImpl<kTokenLessOrEqual>(args);
      break;
    case kTokenGreaterOrEqual:
      BinaryLogicOperatorImpl<kTokenGreaterOrEqual>(args);
      break;
    case kTokenNotEqual:
      BinaryLogicOperatorImpl<kTokenNotEqual>(args);
      break;
    case kTokenGreater:
      BinaryLogicOperatorImpl<kTokenGreater>(args);
      break;
    case kTokenLess:
      BinaryLogicOperatorImpl<kTokenLess>(args);
      break;
    case kTokenAnd:
      BinaryLogicOperatorImpl<kTokenAnd>(args);
      break;
    case kTokenOr:
      BinaryLogicOperatorImpl<kTokenOr>(args);
      break;
    case kTokenNot:
      OperatorLogicNot(args);
      break;
    case kTokenHash:
      CommandHash(args);
      break;
    case kTokenFor:
      CommandForEach(args);
      break;
    case kTokenNullObj:
      CommandNullObj(args);
      break;
    case kTokenDestroy:
      CommandDestroy(args);
      break;
    case kTokenConvert:
      CommandConvert(args);
      break;
    case kTokenRefCount:
      CommandRefCount(args);
      break;
    case kTokenTime:
      CommandTime();
      break;
    case kTokenVersion:
      CommandVersion();
      break;
    case kTokenPatch:
      CommandPatch();
      break;
    case kTokenSwap:
      CommandSwap(args);
      break;
    case kTokenSegment:
      SetSegmentInfo(args);
      break;
    case kTokenBind:
      CommandBind(args);
      break;
    case kTokenExpList:
      ExpList(args);
      break;
    case kTokenInitialArray:
      InitArray(args);
      break;
    case kTokenReturn:
      CommandReturn(args);
      break;
    case kTokenTypeId:
      CommandTypeId(args);
      break;
    case kTokenDir:
      CommandMethods(args);
      break;
    case kTokenExist:
      CommandExist(args);
      break;
    case kTokenFn:
      InitFunctionCatching(args);
      break;
    case kTokenCase:
      CommandCase(args);
      break;
    case kTokenWhen:
      CommandWhen(args);
      break;
    case kTokenAssertR:
      DomainAssert(args);
      break;
    case kTokenEnd:
      switch (worker.mode) {
      case kModeCycle:
      case kModeCycleJump:
        CommandLoopEnd();
        break;
      case kModeCase:
      case kModeCaseJump:
        CommandConditionEnd();
        break;
      case kModeCondition:
      case kModeNextCondition:
        CommandConditionEnd();
        break;
      case kModeForEach:
      case kModeForEachJump:
        CommandForEachEnd();
        break;
      case kModeClosureCatching:
        FinishFunctionCatching(worker_stack_.size() > 1);
        break;
      default:
        break;
      }
      break;
    case kTokenContinue:
    case kTokenBreak:
      CommandContinueOrBreak(token);
      break;
    case kTokenElse:
      CommandElse();
      break;
    case kTokenIf:
    case kTokenElif:
    case kTokenWhile:
      CommandIfOrWhile(token, args);
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
    auto &worker = worker_stack_.top();
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
    auto &worker = worker_stack_.top();
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
      va_base->reserve(temp_list.size());

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
    auto &worker = worker_stack_.top();
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
    Main loop of virtual machine.
    The kisaragi machine runs single command in every single tick of machine loop.
  */
  void Machine::Run() {
    if (ir_stack_.empty()) return;

    bool interface_error = false;
    size_t stop_idx = 0;
    Message msg;
    KIR *ir = ir_stack_.back();
    InterfacePointer interface;
    ObjectMap obj_map;

    worker_stack_.push(MachineWorker());
    obj_stack_.Push();

    MachineWorker *worker = &worker_stack_.top();
    size_t size = ir->size();

    //Refreshing loop tick state to make it work correctly.
    auto refresh_tick = [&]() ->void {
      ir = ir_stack_.back();
      size = ir->size();
      worker = &worker_stack_.top();
    };

    //Enabling skipping by checking current mode value in stack frame.
    auto skipping_checking = [&](CommandPointer &ptr) ->bool {
      msg.Clear();
      switch (worker->mode) {
      case kModeNextCondition:
        Skipping(true, { kTokenElif,kTokenElse });
        break;
      case kModeCaseJump:
        Skipping(true, { kTokenWhen,kTokenElse });
        break;
      case kModeCycleJump:
      case kModeForEachJump:
      case kModeClosureCatching:
        Skipping(false);
        break;
      default:
        break;
      }

      worker->idx += 1;

      ptr = &(*ir)[worker->idx];
      worker->void_call = ptr->first.option.void_call;

      if (worker->error) {
        worker->origin_idx = ptr->first.idx;
      }
      return true;
    };

    auto update_stack_frame = [&](Interface &func)->void {
      ir_stack_.push_back(&func.GetIR());
      worker_stack_.push(MachineWorker());
      obj_stack_.Push();
      obj_stack_.CreateObject(kStrUserFunc, Object(func.GetId()));
      obj_stack_.MergeMap(obj_map);
      obj_stack_.MergeMap(interface->GetClosureRecord());
      refresh_tick();
    };

    //Main loop of virtual machine.
    while (worker->idx < size || worker_stack_.size() > 1) {
      //switch back to last stack frame
      if (worker->idx == size) {
        RecoverLastState();
        refresh_tick();
        worker->RefreshReturnStack(Object());
        worker->idx += 1;
        continue;
      }

      obj_map.clear();
      Command *command = &(*ir)[worker->idx];
      worker->void_call = command->first.option.void_call;

      //Skip commands by checking current machine mode
      if (worker->NeedSkipping()) {
        if (!skipping_checking(command)) break;
      }

      //Embedded machine commands.
      if (command->first.type == kRequestCommand) {
        MachineCommands(command->first.head_command, command->second, command->first);
        
        if (command->first.head_command == kTokenReturn) {
          refresh_tick();
        }

        if (worker->error) {
          worker->origin_idx = command->first.idx;
          break;
        }

        worker->idx += 1;
        continue;
      }

      //Querying function(Interpreter built-in or user-defined)
      if (command->first.type == kRequestInterface) {
        if (!FetchInterface(interface, command, obj_map)) {
          break;
        }
      }

      //Building object map for function call expressed by command
      GenerateArgs(*interface, command->second, obj_map);

      if (worker->error) {
        worker->origin_idx = command->first.idx;
        break;
      }

      //(For user-defined function)
      //Machine will create new stack frame and push IR pointer to machine stack,
      //and start new processing in next tick.
      if (interface->GetPolicyType() == kInterfaceKIR) {
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
        auto arg = BuildStringVector(msg.GetDetail());
        if (!_FetchInterface(interface, arg[0], arg[1])) {
          break;
        }

        if (interface->GetPolicyType() == kInterfaceKIR) {
          update_stack_frame(*interface);
        }
        else {
          msg = interface->Start(obj_map);
          worker->idx += 1;
        }
        continue;
      }

      worker->RefreshReturnStack(msg.GetObj());
      worker->idx += 1;
    }

    if (worker->error) {
      trace::AddEvent(
        Message(kCodeBadExpression, worker->error_string, kStateError)
          .SetIndex(worker->origin_idx));
    }

    if (interface_error) {
      trace::AddEvent(msg.SetIndex(worker->origin_idx));
    }

    obj_stack_.Pop();
    worker_stack_.pop();
  }
}
#undef ERROR_CHECKING
