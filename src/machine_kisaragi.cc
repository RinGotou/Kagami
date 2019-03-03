#include "machine_kisaragi.h"

namespace kagami {
  IRLoader::IRLoader(const char *src) : health(true) {
    size_t error_counter = 0;
    size_t index_counter = 1;
    wstring buf;
    string temp;
    std::wifstream stream(src);
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
      if (temp.empty()) continue;
      script_buf.push_back(CombinedCodeline(index_counter, temp));
      index_counter += 1;
    }

    for (auto it = script_buf.begin(); it != script_buf.end(); ++it) {
      if (!health) {
        if (error_counter < MAX_ERROR_COUNT) {
          error_counter += 1;
        }
        else {
          trace::AddEvent(Message(kCodeIllegalSymbol, "Too many errors. Analyzing is stopped."));
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
      auto type = kagami::util::GetTokenType(toString(current));
      if (type != TokenType::kTokenTypeBlank && exempt_blank_char) {
        head = count;
        exempt_blank_char = false;
      }
      if (current == '\'' && last != '\\') string_processing = !string_processing;
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

  Object Machine::FetchInterfaceObject(string id, string domain) {
    Object obj;
    auto interface = management::FindInterface(id, domain);
    if (interface.Good()) {
      obj.ManageContent(make_shared<Interface>(interface), kTypeIdFunction);
    }
    return obj;
  }

  Object Machine::FetchObject(Argument &arg, bool checking) {
    ObjectPointer ptr = nullptr;
    string domain_type_id = kTypeIdNull;
    Object obj;
    auto &worker = worker_stack_.top();
    auto &return_stack = worker_stack_.top().return_stack;

    auto fetching = [&](ArgumentType type, bool is_domain)->bool {
      switch (type) {
      case kArgumentNormal:
        obj.ManageContent(make_shared<string>(arg.data), kTypeIdRawString);
        break;

      case kArgumentObjectPool:
        ptr = obj_stack_.Find(is_domain ? arg.domain.data : arg.data);
        if (ptr != nullptr) {
          if (is_domain) {
            domain_type_id = ptr->GetTypeId();
          }
          else {
            obj.CreateRef(*ptr);
          }
        }
        else {
          obj = management::GetConstantObject(is_domain ? arg.domain.data : arg.data);

          if (obj.Get() != nullptr) {
            if (is_domain) domain_type_id = obj.GetTypeId();
            return true;
          }

          obj = is_domain ?
            FetchInterfaceObject(arg.domain.data, kTypeIdNull) :
            FetchInterfaceObject(arg.data, domain_type_id);

          if (obj.Get() == nullptr) {
            worker.MakeError("Object is not found - "
              + (is_domain ? arg.domain.data : arg.data));
            return false;
          }
        }
        break;

      case kArgumentReturningStack:
        if (!return_stack.empty()) {
          if (is_domain) {
            domain_type_id = return_stack.top().GetTypeId();
          }
          else {
            obj = return_stack.top();
            if (!checking) return_stack.pop();
          }
        }
        else {
          worker.MakeError("Can't get object from stack.");
          return false;
        }

        break;

      default:
        break;
      }

      return true;
    };

    if (!fetching(arg.domain.type, true)) return Object();
    if (!fetching(arg.type, false)) return Object();
    return obj;
  }

  void Machine::InitFunctionCatching(ArgumentList args) {
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

      if (fn_string_vec[idx] == kStrVaribale) {
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
          if (scope_record.find(unit.first) != scope_record.end()) {
            scope_record.insert_pair(unit.first,
              Object(management::type::GetObjectCopy(unit.second), 
                unit.second.GetTypeId()));
          }
        }
      }

      interface.SetClosureRecord(scope_record);
    }

    if (closure) {
      obj_stack_.CreateObject(fn_string_vec[0],
        Object(make_shared<Interface>(interface), kTypeIdFunction));
    }
    else {
      management::CreateNewInterface(interface);
    }
  }

  void Machine::Skipping(bool enable_terminators, 
    std::initializer_list<GenericToken> terminators) {
    auto &worker = worker_stack_.top();
    size_t nest_counter = 0;
    size_t size = ir_stack_.back()->size();
    bool flag = false;
    auto &ir = *ir_stack_.back();

    if (ir[worker.idx].first.head_command == kTokenEnd && worker.skipping_count == 0) 
      return;

    while (worker.idx < size) {
      Command &command = ir[worker.idx];

      if (command.first.head_command == kTokenSegment) {
        SetSegmentInfo(command.second);

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
      }

      worker.idx += 1;
    }

    if (!flag) {
      worker.MakeError("Expect 'end'");
    }
  }

  void Machine::SetSegmentInfo(ArgumentList args) {
    MachineWorker &worker = worker_stack_.top();
    worker.origin_idx = stoul(args[0].data);
    worker.logic_idx = worker.idx;
    worker.last_command = static_cast<GenericToken>(stol(args[1].data));
  }

  void Machine::CommandIfOrWhile(GenericToken token, ArgumentList args) {
    auto &worker = worker_stack_.top();
    if (args.size() == 1) {
      Object obj = FetchObject(args[0]);
      bool state = false;

      if (obj.GetTypeId() == kTypeIdRawString) {
        string state_str = obj.Cast<string>();

        if (state_str == kStrTrue) {
          state = true;
        }
        else if (state_str == kStrFalse) {
          state = false;
        }
        else {
          auto type = util::GetTokenType(state_str, true);

          switch (type) {
          case kTokenTypeInt:
            state = (stol(state_str) != 0);
            break;
          case kTokenTypeString:
            state = (ParseRawString(state_str).size() > 0);

            break;
          default:
            break;
          }
        }
      }
      else if (obj.GetTypeId() != kTypeIdNull) {
        state = true;
      }

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
        if (!worker.condition_stack.empty()) {
          if (worker.condition_stack.top() == false && worker.mode == kModeNextCondition) {
            worker.mode = kModeCondition;
            worker.condition_stack.top() = true;
          }
        }
        else {
          worker.MakeError("Unexpected Elif.");
          return;
        }
      }
    }
    else {
      worker.MakeError("Too many arguments.");
      return;
    }
  }

  void Machine::CommandElse() {
    auto &worker = worker_stack_.top();
    if (!worker.condition_stack.empty()) {
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
    else {
      worker.MakeError("Unexpected Else.");
      return;
    }
  }

  void Machine::CommandCase(ArgumentList args) {
    auto &worker = worker_stack_.top();

    if (!args.empty()) {
      Object obj = FetchObject(args[0]);
      string type_id = obj.GetTypeId();

      if (!compare(type_id, { kTypeIdRawString,kTypeIdString })){
        worker.MakeError("Non-string object is not supported by case");
        return;
      }

      Object sample_obj(management::type::GetObjectCopy(obj), type_id);
      obj_stack_.Push();
      obj_stack_.CreateObject(kStrCaseObj, sample_obj);
      worker.SwitchToMode(kModeCaseJump);
      worker.condition_stack.push(false);
    }
    else {
      worker.MakeError("Empty argument list");
      return;
    }
  }

  void Machine::CommandWhen(ArgumentList args) {
    auto &worker = worker_stack_.top();
    bool result = false;

    if (worker.condition_stack.empty()) {
      worker.MakeError("Unexpected 'when'");
      return;
    }

    if (!compare(worker.mode, { kModeCase,kModeCaseJump })) {
      worker.MakeError("Unexpected 'when'");
      return;
    }

    if (worker.mode == kModeCase) {
      worker.mode = kModeCaseJump;
      return;
    }
    
    if (worker.condition_stack.top()) {
      return;
    }

    if (!args.empty()) {
      ObjectPointer ptr = obj_stack_.Find(kStrCaseObj);
      string content;
      bool found = false;
      bool error = false;

      if (ptr == nullptr) {
        worker.MakeError("Unexpected 'when'");
        return;
      }

      if (!compare(ptr->GetTypeId(), { kTypeIdRawString,kTypeIdString })) {
        worker.MakeError("Non-string object is not supported by when");
        return;
      }

      content = ptr->Cast<string>();

      for (auto &unit : args) {
        Object obj = FetchObject(unit);

        if (!compare(obj.GetTypeId(), { kTypeIdRawString,kTypeIdString })) {
          worker.MakeError("Non-string object is not supported by when");
          error = true;
          break;
        }

        if (obj.Cast<string>() == content) {
          found = true;
          break;
        }
      }

      if (error) return;

      if (found) {
        worker.mode = kModeCase;
        worker.condition_stack.top() = true;
      }
    }
  }

  void Machine::CommandContinueOrBreak(GenericToken token) {
    auto &worker = worker_stack_.top();
    while (!worker.mode_stack.empty() && worker.mode != kModeCycle) {
      if (worker.mode == kModeCondition || worker.mode == kModeCase) {
        worker.condition_stack.pop();
        worker.skipping_count += 1;
      }

      worker.GoLastMode();
    }

    if (worker.mode == kModeCycle) {
      worker.mode = kModeCycleJump;
      switch (token) {
      case kTokenContinue:worker.activated_continue = true; break;
      case kTokenBreak:worker.activated_break = true; break;
      default:break;
      }
    }
    else {
      worker.MakeError("Invalid 'continue'");
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

  void Machine::CommandBind(ArgumentList args) {
    auto &worker = worker_stack_.top();
    //Do not change the order!
    auto src = FetchObject(args[1]);
    auto dest = FetchObject(args[0]);
     
    if (dest.IsRef()) {
      dest.ManageContent(management::type::GetObjectCopy(src), src.GetTypeId());
      return;
    }

    string id = dest.Cast<string>();
    ObjectPointer ptr = obj_stack_.Find(id);
    
    if (ptr != nullptr) {
      ptr->ManageContent(management::type::GetObjectCopy(src), src.GetTypeId());
      return;
    }

    if (util::GetTokenType(id, true) != kTokenTypeGeneric) {
      worker.MakeError("Invalid object id");
      return;
    }

    Object obj(management::type::GetObjectCopy(src), src.GetTypeId());

    if (!obj_stack_.CreateObject(id, obj)) {
      worker.MakeError("Object creation failed");
      return;
    }
  }

  void Machine::CommandTypeId(ArgumentList args) {
    auto &worker = worker_stack_.top();

    if (args.size() > 1) {
      shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

      for (auto &unit : args) {
        base->emplace_back(Object(FetchObject(unit).GetTypeId()));
      }

      Object obj(base, kTypeIdArray);
      obj.SetConstructorFlag();
      worker.return_stack.push(obj);
    }
    else if (args.size() == 1) {
      worker.return_stack.push(Object(FetchObject(args[0]).GetTypeId()));
    }
    else {
      worker.return_stack.push(Object(kTypeIdNull));
    }
  }

  void Machine::CommandMethods(ArgumentList args) {
    auto &worker = worker_stack_.top();

    if (args.size() != 1) {
      worker.MakeError("Argument error");
      return;
    }

    Object obj = FetchObject(args[0]);
    auto methods = management::type::GetMethods(obj.GetTypeId());
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

    for (auto &unit : methods) {
      base->emplace_back(Object(make_shared<string>(unit), kTypeIdString));
    }

    Object ret_obj(base, kTypeIdArray);
    ret_obj.SetConstructorFlag();
    worker.return_stack.push(ret_obj);
  }

  void Machine::CommandExist(ArgumentList args) {
    auto &worker = worker_stack_.top();

    if (args.size() != 2) {
      worker.MakeError("Argument error");
    }

    //Do not change the order
    Object str_obj = FetchObject(args[1]);
    Object obj = FetchObject(args[0]);
    
    if (!compare(str_obj.GetTypeId(), { kTypeIdRawString,kTypeIdString })) {
      worker.MakeError("Invalid method id");
      return;
    }

    string str = ParseRawString(str_obj.Cast<string>());
    auto methods = management::type::GetMethods(obj.GetTypeId());
    Object ret_obj(util::MakeBoolean(find_in_vector(str, methods)));

    worker.return_stack.push(ret_obj);
  }

  void Machine::ExpList(ArgumentList args) {
    auto &worker = worker_stack_.top();
    if (!args.empty()) {
      worker.return_stack.push(FetchObject(args.back()));
    }
  }

  void Machine::InitArray(ArgumentList args) {
    auto &worker = worker_stack_.top();
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

    if (!args.empty()) {
      for (auto &unit : args) {
        base->emplace_back(FetchObject(unit));
      }
    }

    Object obj(base, kTypeIdArray);
    obj.SetConstructorFlag();
    worker.return_stack.push(obj);
  }

  void Machine::DomainAssert(ArgumentList args, bool returning, bool no_feeding) {
    auto &worker = worker_stack_.top();
    Object obj = FetchObject(args[0], no_feeding);
    string id = FetchObject(args[1]).Cast<string>();
    auto methods = management::type::GetMethods(obj.GetTypeId());
    bool result = find_in_vector(id, methods);

    if (!result) {
      worker.MakeError("Method/member is not found - " + id);
      return;
    }

    if (returning) {
      auto interface = management::FindInterface(id, obj.GetTypeId());

      if (!interface.Good()) {
        worker.MakeError("Method is not found - " + id);
        return;
      }

      Object ret_obj(make_shared<Interface>(interface), kTypeIdFunction);
      worker.return_stack.push(ret_obj);
    }
  }

  void Machine::CommandReturn(ArgumentList args) {
    if (worker_stack_.size() <= 1) {
      trace::AddEvent(Message(kCodeBadExpression, "Unexpected return.", kStateError));
    }
    else if (args.size() == 1) {
      Object src_obj = FetchObject(args[0]);
      Object ret_obj(management::type::GetObjectCopy(src_obj), src_obj.GetTypeId());
      RecoverLastState();
      worker_stack_.top().return_stack.push(ret_obj);
    }
    else if (args.size() == 0) {
      RecoverLastState();
      worker_stack_.top().return_stack.push(Object());
    }
    else {
      shared_ptr<ObjectArray> obj_array(make_shared<ObjectArray>());
      for (auto it = args.begin(); it != args.end(); ++it) {
        obj_array->emplace_back(FetchObject(*it));
      }
      Object ret_obj(obj_array, kTypeIdArray);
      RecoverLastState();
      worker_stack_.top().return_stack.push(ret_obj);
    }
  }

  void Machine::MachineCommands(GenericToken token, ArgumentList args, Request request) {
    auto &worker = worker_stack_.top();

    switch (token) {
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
    case kTokenAssert:
    case kTokenAssertR:
      DomainAssert(args, token == kTokenAssertR, request.option.no_feeding);
      break;
    case kTokenEnd:
      if (worker.mode == kModeCycle || worker.mode == kModeCycleJump) {
        CommandLoopEnd();
      }
      else if (worker.mode == kModeCase || worker.mode == kModeCaseJump) {
        CommandConditionEnd();
      }
      else if (worker.mode == kModeCondition || worker.mode == kModeNextCondition) {
        CommandConditionEnd();
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

  void Machine::GenerateArgs(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
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

  void Machine::Generate_Normal(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
    auto &worker = worker_stack_.top();
    auto params = interface.GetParameters();

    if (args.size() > params.size()) {
      worker.MakeError("Too many arguments");
      return;
    }

    if (args.size() < params.size()) {
      worker.MakeError("Required argument count is " +
        to_string(params.size()) +
        ", but provided argument count is " +
        to_string(args.size()) + ".");
      return;
    }

    for (auto it = params.rbegin(); it != params.rend(); ++it) {
      obj_map.insert(NamedObject(*it, FetchObject(args.back())));
      args.pop_back();
    }
  }

  void Machine::Generate_AutoSize(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
    auto &worker = worker_stack_.top();
    auto params = interface.GetParameters();
    list<Object> temp_list;
    shared_ptr<ObjectArray> va_base(new ObjectArray());

    if (args.size() < params.size()) {
      worker.MakeError("Too few arguments.");
      return;
    }

    while (args.size() >= params.size() - 1 && !args.empty()) {
      temp_list.emplace_front(FetchObject(args.back()));
      args.pop_back();
    }

    if (!temp_list.empty()) {
      for (auto it = temp_list.begin(); it != temp_list.end(); ++it) {
        va_base->emplace_back(*it);
      }
    }

    temp_list.clear();

    obj_map.insert(NamedObject(params.back(), Object(va_base, kTypeIdArray)));
    
    auto it = ++params.rbegin();

    if (!args.empty()) {
      for (; it != params.rend(); ++it) {
        obj_map.insert(NamedObject(*it, FetchObject(args.back())));
        args.pop_back();
      }
    }
  }

  void Machine::Generate_AutoFill(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
    auto &worker = worker_stack_.top();
    auto params = interface.GetParameters();
    size_t min_size = interface.GetMinArgSize();

    if (args.size() > params.size()) {
      worker.MakeError("Too many arguments");
      return;
    }

    if (args.size() < min_size) {
      worker.MakeError("Required minimum argument count is" +
        to_string(min_size) +
        ", but provided argument count is" +
        to_string(args.size()));
      return;
    }

    while (args.size() != params.size()) {
      obj_map.insert(NamedObject(params.back(), Object()));
      params.pop_back();
    }

    for (auto it = params.rbegin(); it != params.rend(); ++it) {
      obj_map.insert(NamedObject(*it, FetchObject(args.back())));
      args.pop_back();
    }
  }

  void Machine::Preprocessor() {
    if (ir_stack_.empty()) return;
    KIR &ir = *ir_stack_.back();
    size_t size = ir.size();
    size_t nest_counter = 0;
    bool catching = false;
    bool error = false;
    
    worker_stack_.push(MachineWorker());
    MachineWorker &worker = worker_stack_.top();
    map<size_t, size_t> catched_block;

    while (worker.idx < size) {
      Command &command = ir[worker.idx];

      if (command.first.head_command == kTokenFn) {
        if (!catching) {
          if (nest_counter > 0) {
            worker.MakeError("Invalid function definition");
            break;
          }
        }

        if (catching) {
          nest_counter += 1; //ignore closure block
          worker.idx += 1;
          continue;
        }

        InitFunctionCatching(command.second);
        catching = true;
        worker.idx += 1;
        continue;
      }
      else if (command.first.head_command == kTokenEnd) {
        if (nest_counter > 0) {
          nest_counter -= 1;
          worker.idx += 1;
          continue;
        }

        catched_block.insert(std::make_pair(worker.fn_idx - 1, worker.idx));
        FinishFunctionCatching();
        catching = false;
      }
      else {
        if (find_in_vector(command.first.head_command, nest_flag_collection)) {
          nest_counter += 1;
        }
      }

      worker.idx += 1;
    }

    if (catching) {
      worker.MakeError("Expect 'end'.");
    }

    if (worker.error) {
      trace::AddEvent(Message(kCodeBadExpression, worker.error_string, kStateError));
    }
    else {
      KIR not_catched;

      for (size_t idx = 0; idx < ir.size(); idx += 1) {
        auto it = catched_block.find(idx);

        if (it != catched_block.end()) {
          idx = it->second;
          continue;
        }

        not_catched.emplace_back(ir[idx]);
      }

      ir.swap(not_catched);

      worker_stack_.pop();
    }
  }

  void Machine::Run() {
    if (ir_stack_.empty()) return;
    StateLevel level = kStateNormal;
    StateCode code = kCodeSuccess;
    bool interface_error = false;
    string detail;
    string type_id = kTypeIdNull;
    Message msg;
    KIR *ir = ir_stack_.back();
    Interface interface;
    ObjectMap obj_map;

    worker_stack_.push(MachineWorker());
    obj_stack_.Push();

    MachineWorker *worker = &worker_stack_.top();
    size_t size = ir->size();

    auto refresh_tick = [&]() ->void {
      ir = ir_stack_.back();
      size = ir->size();
      worker = &worker_stack_.top();
    };

    //Main loop of current thread
    while (worker->idx < size || worker_stack_.size() > 1) {
      if (worker->idx == size) {
        RecoverLastState();
        refresh_tick();
        worker->return_stack.push(Object());
        worker->idx += 1;
        continue;
      }

      obj_map.clear();
      Command *command = &(*ir)[worker->idx];

      if (worker->NeedSkipping()) {
        msg.Clear();
        switch (worker->mode) {
        case kModeNextCondition:
          Skipping(true, { kTokenElif,kTokenElse });
          break;
        case kModeCaseJump:
          Skipping(true, { kTokenWhen,kTokenElse });
          break;
        case kModeCycleJump:
        case kModeClosureCatching:
          Skipping(false);
          break;
        default:
          break;
        }

        worker->idx += 1;

        command = &(*ir)[worker->idx];

        if (worker->error) break;
      }

      if (command->first.type == kRequestCommand 
        && !util::IsOperator(command->first.head_command)) {
        MachineCommands(command->first.head_command, command->second, command->first);

        if (command->first.head_command == kTokenReturn) {
          refresh_tick();
        }

        if (worker->deliver) {
          msg = worker->GetMsg();
          worker->msg.Clear();
        }

        if (worker->error) break;

        worker->idx += 1;
        continue;
      }
      
      if (command->first.type == kRequestCommand) {
        interface = management::GetGenericInterface(command->first.head_command);
      }

      if (command->first.type == kRequestInterface) {
        if (command->first.domain.type != kArgumentNull) {
          Object domain_obj = FetchObject(command->first.domain, true);
          type_id = domain_obj.GetTypeId();
          interface = management::FindInterface(command->first.head_interface, type_id);
          obj_map.insert(NamedObject(kStrObject, domain_obj));
        }
        else {
          interface = management::FindInterface(command->first.head_interface);

          if (!interface.Good()) {
            ObjectPointer func_obj_ptr = obj_stack_.Find(command->first.head_interface);
            if (func_obj_ptr != nullptr && func_obj_ptr->GetTypeId() == kTypeIdFunction) {
              interface = func_obj_ptr->Cast<Interface>();
            }
            else {
              worker->MakeError(command->first.head_interface + " is not a function object.");
            }
          }
        }
      }

      if (worker->error) break;

      if (!interface.Good()) {
        worker->MakeError("Function is not found - " + command->first.head_interface);
      }

      GenerateArgs(interface, command->second, obj_map);

      if (worker->error) break;

      if (interface.GetPolicyType() == kInterfaceKIR) {
        ir_stack_.push_back(&interface.GetIR());
        worker_stack_.push(MachineWorker());
        obj_stack_.Push();
        obj_stack_.CreateObject(kStrUserFunc, Object(command->first.head_interface));
        obj_stack_.MergeMap(obj_map);
        obj_stack_.MergeMap(interface.GetClosureRecord());
        ir = ir_stack_.back();
        size = ir->size();
        worker = &worker_stack_.top();
        continue;
      }
      else {
        msg = interface.Start(obj_map);
      }

      if (msg.GetLevel() == kStateError) {
        interface_error = true;
        break;
      }

      if (msg.GetCode() == kCodeObject) {
        worker->return_stack.push(msg.GetObj());
      }

      worker->idx += 1;
    }

    if (worker->error) {
      trace::AddEvent(
        Message(kCodeBadExpression, worker->error_string, kStateError)
          .SetIndex(worker->origin_idx)
      );
    }

    if (interface_error) {
      trace::AddEvent(msg);
    }

    obj_stack_.Pop();
    worker_stack_.pop();
  }

  /* string/wstring convertor */
#if defined(_WIN32) && defined(_MSC_VER)
  //from MSDN
  std::wstring s2ws(const std::string& s) {
    auto slength = static_cast<int>(s.length()) + 1;
    auto len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    auto *buf = new wchar_t[len];
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
    std::wstring r(buf);
    delete[] buf;
    return r;
  }

  std::string ws2s(const std::wstring& s) {
    int len;
    int slength = (int)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
    return r;
  }
#else
  //from https://www.yasuhisay.info/interface/20090722/1248245439
  std::wstring s2ws(const std::string& s) {
    if (s.empty()) return wstring();
    size_t length = s.size();
    wchar_t *wc = (wchar_t*)malloc(sizeof(wchar_t)* (length + 2));
    mbstowcs(wc, s.c_str(), s.length() + 1);
    std::wstring str(wc);
    free(wc);
    return str;
  }

  std::string ws2s(const std::wstring& s) {
    if (s.empty()) return string();
    size_t length = s.size();
    char *c = (char*)malloc(sizeof(char)* length * 2);
    wcstombs(c, s.c_str(), s.length() + 1);
    std::string result(c);
    free(c);
    return result;
  }
#endif

  bool IsStringObject(Object &obj) {
    auto id = obj.GetTypeId();
    return (id == kTypeIdRawString || id == kTypeIdString);
  }

  string ParseRawString(const string &src) {
    string result = src;
    if (util::IsString(result)) result = util::GetRawString(result);
    return result;
  }
}