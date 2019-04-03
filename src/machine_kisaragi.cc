#include "machine_kisaragi.h"

namespace kagami {
  IRLoader::IRLoader(const char *src) : health(true) {
    bool comment_block = false;
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

  Object Machine::FetchPlainObject(string value) {
    auto type = util::GetTokenType(value, true);
    Object obj;
    switch (type) {
    case kTokenTypeInt:
      obj.ManageContent(make_shared<long>(stol(value)), kTypeIdInt); 
      break;
    case kTokenTypeFloat:
      obj.ManageContent(make_shared<double>(stod(value)), kTypeIdFloat);
      break;
    case kTokenTypeBool:
      obj.ManageContent(make_shared<bool>(value == kStrTrue), kTypeIdBool);
      break;
    case kTokenTypeString:
      obj.ManageContent(make_shared<string>(ParseRawString(value)), kTypeIdString);
      break;
    case kTokenTypeGeneric:
      obj.ManageContent(make_shared<string>(value), kTypeIdString);
      break;
    default:
      break;
    }

    return obj;
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
        obj = FetchPlainObject(arg.data);
        break;

      case kArgumentObjectStack:
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

      case kArgumentReturnStack:
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

  bool Machine::_FetchInterface(Interface &interface, string id, string type_id) {
    auto &worker = worker_stack_.top();

    //Modified version for function invoking
    if (type_id != kTypeIdNull) {
      if (!find_in_vector(id, management::type::GetMethods(type_id))) {
        worker.MakeError("Method is not found - " + id);
        return false;
      }

      interface = management::FindInterface(id, type_id);
      return true;
    }
    else {
      interface = management::FindInterface(id);

      if (interface.Good()) return true;

      ObjectPointer ptr = obj_stack_.Find(id);

      if (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction) {
        interface = ptr->Cast<Interface>();
        return true;
      }

      worker.MakeError("Function is not found - " + id);
    }

    return false;
  }

  bool Machine::FetchInterface(Interface &interface, CommandPointer &command, ObjectMap &obj_map) {
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

      if (!find_in_vector(id, management::type::GetMethods(obj.GetTypeId()))) {
        worker.MakeError("Method is not found - " + id);
        return false;
      }

      interface = management::FindInterface(id, obj.GetTypeId());
      obj_map.insert(NamedObject(kStrObject, obj));
      return true;
    }
    //Plain bulit-in function and user-defined function
    //At first, Machine will querying in built-in function map,
    //and then try to fetch function object in heap.
    else {
      interface = management::FindInterface(id);

      if (interface.Good()) return true;

      ObjectPointer ptr = obj_stack_.Find(id);

      if (ptr != nullptr && ptr->GetTypeId() == kTypeIdFunction) {
        interface = ptr->Cast<Interface>();
        return true;
      }

      worker.MakeError("Function is not found - " + id);
    }

    return false;
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
            scope_record.insert_pair(unit.first,
              Object(management::type::GetObjectCopy(unit.second), 
                unit.second.GetTypeId()));
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

  Message Machine::Invoke(Object obj, string id, 
    const initializer_list<NamedObject> &&args,
    InvokingRecoverPoint recover_point) {
    auto &worker = worker_stack_.top();
    auto methods = management::type::GetMethods(obj.GetTypeId());
    auto found = find_in_vector(id, methods);
    Interface interface;

    if (recover_point != nullptr) {
      worker.recover_point = recover_point;
    }

    if (!found) {
      //Immediately push event to avoid ugly checking block.
      trace::AddEvent("Method is not found" + id);
      return Message();
    }

    _FetchInterface(interface, id, obj.GetTypeId());

    if (interface.GetPolicyType() == kInterfaceKIR) {
      worker.invoking_point = true;
      worker.invoking_dest.reset(new Interface(interface));
    }
    else {
      ObjectMap obj_map = args;
      obj_map.insert(NamedObject(kStrObject, obj));

      return interface.Start(obj_map);
    }

    return Message();
  }

  void Machine::SetSegmentInfo(ArgumentList args) {
    MachineWorker &worker = worker_stack_.top();
    worker.origin_idx = stoul(args[0].data);
    worker.logic_idx = worker.idx;
    worker.last_command = static_cast<GenericToken>(stol(args[1].data));
  }

  void Machine::CommandSwap(ArgumentList args) {
    auto &worker = worker_stack_.top();
    auto &right = FetchObject(args[1]).Deref();
    auto &left = FetchObject(args[0]).Deref();

    left.swap(right);
  }

  void Machine::CommandIfOrWhile(GenericToken token, ArgumentList args) {
    auto &worker = worker_stack_.top();
    if (args.size() == 1) {
      Object obj = FetchObject(args[0]);

      if (obj.GetTypeId() != kTypeIdBool) {
        worker.MakeError("Invalid state value.");
        return;
      }

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

  void Machine::CommandForEach(ArgumentList args) {
    auto &worker = worker_stack_.top();
    ObjectMap obj_map;

    if (!worker.loop_head.empty() && worker.loop_head.top() == worker.logic_idx - 1) {
      ForEachChecking(args);
      return;
    }

    auto unit_id = FetchObject(args[0]).Cast<string>();
    auto container_obj = FetchObject(args[1]);

    auto methods = management::type::GetMethods(container_obj.GetTypeId());

    if (!management::type::CheckBehavior(container_obj, kContainerBehavior)) {
      worker.MakeError("Invalid object container");
      return;
    }

    auto msg = Invoke(container_obj, kStrHead);

    if (msg.GetCode() != kCodeObject) {
      worker.MakeError("Invalid iterator of container");
      return;
    }

    auto iterator_obj = msg.GetObj();
    
    if (!management::type::CheckBehavior(iterator_obj, kIteratorBehavior)) {
      worker.MakeError("Invalid iterator behavior");
      return;
    }

    auto unit = Invoke(iterator_obj, "get").GetObj();

    obj_stack_.Push();
    obj_stack_.CreateObject("!iterator", iterator_obj);
    obj_stack_.CreateObject(unit_id, unit);
    worker.loop_head.push(worker.logic_idx - 1);
    worker.SwitchToMode(kModeForEach);
  }

  void Machine::ForEachChecking(ArgumentList args) {
    using namespace management;
    auto &worker = worker_stack_.top();
    auto unit_id = FetchObject(args[0]).Cast<string>();
    auto iterator = *obj_stack_.GetCurrent().Find("!iterator");
    auto container = FetchObject(args[1]);
    auto method_compare = FindInterface(kStrCompare, iterator.GetTypeId());
    auto method_tail = FindInterface("tail", container.GetTypeId());
    auto method_step_forward = FindInterface("step_forward", iterator.GetTypeId());
    ObjectMap obj_map;

    auto tail = Invoke(container, kStrTail).GetObj();

    if (!type::CheckBehavior(tail, kIteratorBehavior)) {
      worker.MakeError("Invalid container behavior");
      return;
    }

    Invoke(iterator, "step_forward");

    auto result = Invoke(iterator, kStrCompare,
      { NamedObject(kStrRightHandSide,tail) }).GetObj();

    if (result.GetTypeId() != kTypeIdBool) {
      worker.MakeError("Invalid iterator behavior");
      return;
    }

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

      if (!util::IsPlainType(type_id)) {
        worker.MakeError("Non-plain object is not supported by case");
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
      string type_id = ptr->GetTypeId();
      bool found = false;

      if (ptr == nullptr) {
        worker.MakeError("Unexpected 'when'");
        return;
      }

      if (!util::IsPlainType(type_id)) {
        worker.MakeError("Non-plain object is not supported by when");
        return;
      }

      for (auto it = args.rbegin(); it != args.rend(); ++it) {
        Object obj = FetchObject(*it);

        if (obj.GetTypeId() != type_id) continue;

        if (type_id == kTypeIdInt) {
          found = (ptr->Cast<long>() == obj.Cast<long>());
        }
        else if (type_id == kTypeIdFloat) {
          found = (ptr->Cast<double>() == obj.Cast<double>());
        }
        else if (type_id == kTypeIdString) {
          found = (ptr->Cast<string>() == obj.Cast<string>());
        }
        else if (type_id == kTypeIdBool) {
          found = (ptr->Cast<bool>() == obj.Cast<bool>());
        }

        if (found) break;
      }

      if (found) {
        worker.mode = kModeCase;
        worker.condition_stack.top() = true;
      }
    }
  }

  void Machine::CommandContinueOrBreak(GenericToken token) {
    auto &worker = worker_stack_.top();
    while (!worker.mode_stack.empty() 
      && (worker.mode != kModeCycle || worker.mode != kModeForEach)) {
      if (worker.mode == kModeCondition || worker.mode == kModeCase) {
        worker.condition_stack.pop();
        worker.skipping_count += 1;
      }

      worker.GoLastMode();
    }

    if (worker.mode == kModeCycle || worker.mode == kModeForEach) {
      worker.mode = kModeCycleJump;
      switch (token) {
      case kTokenContinue:worker.activated_continue = true; break;
      case kTokenBreak:worker.activated_break = true; break;
      default:break;
      }
    }
    else {
      worker.MakeError("Invalid 'continue'/'break'");
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

  void Machine::CommandForEachEnd() {
    auto &worker = worker_stack_.top();
    if (worker.mode == kModeForEach) {
      if (worker.loop_tail.empty() || worker.loop_tail.top() != worker.logic_idx - 1) {
        worker.loop_tail.push(worker.logic_idx - 1);
      }
      worker.idx = worker.loop_head.top();
      obj_stack_.GetCurrent().ClearExcept("!iterator");
    }
    else if (worker.mode == kModeForEachJump) {
      if (worker.activated_continue) {
        if (worker.loop_tail.empty() || worker.loop_tail.top() != worker.logic_idx - 1) {
          worker.loop_tail.push(worker.logic_idx - 1);
        }
        worker.idx = worker.loop_head.top();
        worker.mode = kModeForEach;
        worker.activated_continue = false;
        obj_stack_.GetCurrent().ClearExcept("!iterator");
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
    
    if (str_obj.GetTypeId() != kTypeIdString) {
      worker.MakeError("Invalid method id");
      return;
    }

    string str = str_obj.Cast<string>();
    auto methods = management::type::GetMethods(obj.GetTypeId());
    Object ret_obj(make_shared<bool>(find_in_vector(str, methods)), kTypeIdBool);

    worker.return_stack.push(ret_obj);
  }

  void Machine::CommandNullObj(ArgumentList args) {
    auto &worker = worker_stack_.top();
    if (args.size() != 1) {
      worker.MakeError("Invalid argument of null()");
      return;
    }

    Object obj = FetchObject(args[0]);
    worker.return_stack.push(
      Object(make_shared<bool>(obj.GetTypeId() == kTypeIdNull), kTypeIdBool));
  }

  void Machine::CommandDestroy(ArgumentList args) {
    auto &worker = worker_stack_.top();
    if (args.size() != 1) {
      worker.MakeError("Invalid argument of destroy()");
      return;
    }
    Object &obj = FetchObject(args[0]).Deref();
    obj.swap(Object());
  }

  void Machine::CommandConvert(ArgumentList args) {
    auto &worker = worker_stack_.top();
    if (args.size() != 1) {
      worker.MakeError("Invalid argument of convert()");
      return;
    }

    auto &arg = args[0];
    if (arg.type == kArgumentNormal) {
      FetchPlainObject(arg.data);
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
          ret_obj.ManageContent(make_shared<long>(stol(str)), kTypeIdInt);
          break;
        case kTokenTypeFloat:
          ret_obj.ManageContent(make_shared<double>(stod(str)), kTypeIdFloat);
          break;
        case kTokenTypeBool:
          ret_obj.ManageContent(make_shared<bool>(str == kStrTrue), kTypeIdBool);
          break;
        default:
          ret_obj = obj;
          break;
        }
      }
      else {
        if (find_in_vector(kStrGetStr, management::type::GetMethods(type_id))) {
          auto ret_obj = Invoke(obj, kStrGetStr).GetObj();
        }
        else {
          worker.MakeError("Invalid argument of convert()");
          return;
        }
      }

      worker.return_stack.push(ret_obj);
    }
  }

  void Machine::CommandRefCount(ArgumentList args) {
    auto &worker = worker_stack_.top();

    if (args.size() != 1) {
      worker.MakeError("Invalid argument of ref_count()");
      return;
    }
    auto &obj = FetchObject(args[0]).Deref();
    Object ret_obj(make_shared<long>(obj.ObjRefCount()), kTypeIdInt);

    worker.return_stack.push(ret_obj);
  }

  void Machine::CommandTime() {
    auto &worker = worker_stack_.top();
    time_t now = time(nullptr);
    string nowtime(ctime(&now));
    nowtime.pop_back();
    worker.return_stack.push(Object(nowtime));
  }

  void Machine::CommandVersion() {
    auto &worker = worker_stack_.top();
    worker.return_stack.push(Object(kInterpreterVersion));
  }

  void Machine::CommandPatch() {
    auto &worker = worker_stack_.top();
    worker.return_stack.push(Object(kPatchName));
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
      return;
    }

    auto *container = &obj_stack_.GetCurrent();
    while (container->Find(kStrUserFunc) == nullptr) {
      obj_stack_.Pop();
      container = &obj_stack_.GetCurrent();
    }

    if (args.size() == 1) {
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
    case kTokenAssert:
    case kTokenAssertR:
      DomainAssert(args, token == kTokenAssertR, request.option.no_feeding);
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

  /*
    Main loop of virtual machine.
    The kisaragi machine runs single command in every single tick of machine loop.
  */
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

      if (worker->error) return false;
      return true;
    };

    auto update_stack_frame = [&](Interface &func)->void {
      ir_stack_.push_back(&func.GetIR());
      worker_stack_.push(MachineWorker());
      obj_stack_.Push();
      obj_stack_.CreateObject(kStrUserFunc, Object(func.GetId()));
      obj_stack_.MergeMap(obj_map);
      obj_stack_.MergeMap(interface.GetClosureRecord());
      refresh_tick();
    };

    //Main loop of virtual machine.
    while (worker->idx < size || worker_stack_.size() > 1) {
      //switch back to last stack frame
      if (worker->idx == size) {
        RecoverLastState();
        refresh_tick();
        worker->return_stack.push(Object());
        worker->idx += 1;
        continue;
      }

      obj_map.clear();
      Command *command = &(*ir)[worker->idx];

      //Skip commands by checking current machine mode
      if (worker->NeedSkipping()) {
        if (!skipping_checking(command)) break;
      }

      //Embedded machine commands.
      if (command->first.type == kRequestCommand 
        && !util::IsOperator(command->first.head_command)) {
        if (worker->invoking_point) {
          (this->*(worker->recover_point))(command->second);
        }
        else {
          MachineCommands(command->first.head_command, 
            command->second, command->first);
        }

        if (command->first.head_command == kTokenReturn) {
          refresh_tick();
        }

        if (worker->error) break;

        if (worker->invoking_point) {
          if (worker->recover_point == nullptr) {
            worker->invoking_point = false;
            worker->recover_point = nullptr;
            worker->invoking_dest.reset();
          }
          else {
            update_stack_frame(*worker->invoking_dest);
            continue;
          }
        }

        worker->idx += 1;
        continue;
      }
      
      //For built-in operator functions.
      if (command->first.type == kRequestCommand) {
        interface = management::GetGenericInterface(command->first.head_command);
      }

      //Querying function(Interpreter built-in or user-defined)
      if (command->first.type == kRequestInterface) {
        if (!FetchInterface(interface, command, obj_map)) {
          break;
        }
      }

      //Building object map for function call expressed by command
      GenerateArgs(interface, command->second, obj_map);

      if (worker->error) break;

      //(For user-defined function)
      //Machine will create new stack frame and push IR pointer to machine stack,
      //and start new processing in next tick.
      if (interface.GetPolicyType() == kInterfaceKIR) {
        update_stack_frame(interface);
        continue;
      }
      else {
        msg = interface.Start(obj_map);
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

        if (interface.GetPolicyType() == kInterfaceKIR) {
          update_stack_frame(interface);
        }
        else {
          msg = interface.Start(obj_map);
          worker->idx += 1;
        }
        continue;
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
      trace::AddEvent(msg.SetIndex(worker->origin_idx));
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

  string ParseRawString(const string &src) {
    string result = src;
    if (util::IsString(result)) result = util::GetRawString(result);
    return result;
  }
}
