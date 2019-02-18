#include "module.h"

namespace kagami {
  shared_ptr<void> FakeCopy(shared_ptr<void> target) {
    return target;
  }

  string ParseRawString(const string &src) {
    string result = src;
    if (util::IsString(result)) result = util::GetRawString(result);
    return result;
  }

  void CopyObject(Object &dest, Object &src) {
    dest.ManageContent(management::type::GetObjectCopy(src), src.GetTypeId());
  }

  bool IsStringObject(Object &obj) {
    auto id = obj.GetTypeId();
    return (id == kTypeIdRawString || id == kTypeIdString);
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

  /* Packaging interface into object and act as a object */
  inline Object GetFunctionObject(string id, string domain) {
    Object obj;
    auto interface = management::FindInterface(id, domain);
    if (interface.Good()) {
      obj.ManageContent(make_shared<Interface>(interface), kTypeIdFunction);
    }
    return obj;
  }

  /* Disposing all indentation and comments */
  string IndentationAndCommentProc(string target) {
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

  /* Combing line */
  vector<StringUnit> MultilineProcessing(vector<string> &src) {
    vector<StringUnit> output;
    string buf;
    size_t idx = 0, line_idx = 0;

    auto isInString = [](string src) {
      bool inString = false;
      bool escape = false;
      for (size_t strIdx = 0; strIdx < src.size(); ++strIdx) {
        if (inString && escape) {
          escape = false;
          continue;
        }
        if (src[strIdx] == '\'') inString = !inString;
        if (inString && !escape && src[strIdx] == '\\') {
          escape = true;
          continue;
        }
      }
      return inString;
    };

    while (idx < src.size()) {
      buf = IndentationAndCommentProc(src[idx]);
      if (buf == "") {
        idx += 1;
        continue;
      }
      line_idx = idx;
      while (buf.back() == '_') {
        bool inString = isInString(buf);
        if (!inString) {
          idx += 1;
          buf.pop_back();
          buf = buf + IndentationAndCommentProc(src[idx]);
        }
      }
      output.push_back(StringUnit(line_idx, buf));
      idx += 1;
    }

    return output;
  }

  /* 
     Pack IR vector into a new module and run it.
  */
  Message FunctionAgentTunnel(ObjectMap &p, vector<IR> storage) {
    return Module(storage).RunAsFunction(p);
  }

  inline bool GetBooleanValue(string src) {
    if (src == kStrTrue) return true;
    if (src == kStrFalse) return false;
    if (src == "0" || src.empty()) return false;
    return true;
  }

  void MachCtlBlk::Case() {
    management::CreateContainer();
    mode_stack.push(mode);
    mode = kModeCaseJump;
    condition_stack.push(false);
  }

  void MachCtlBlk::When(bool value) {
    if (!condition_stack.empty()) {
      if (mode == kModeCase && condition_stack.top() == true) {
        mode = kModeCaseJump;
      }
      else if (value == true && condition_stack.top() == false) {
        mode = kModeCase;
        condition_stack.top() = true;
      }
    }

  }

  void MachCtlBlk::ConditionIf(bool value) {
    mode_stack.push(mode);
    management::CreateContainer();
    mode = value ? kModeCondition : kModeNextCondition;
    condition_stack.push(value);
  }

  void MachCtlBlk::ConditionElif(bool value) {
    if (!condition_stack.empty()) {
      if (condition_stack.top() == false && mode == kModeNextCondition) {
        management::CreateContainer();
        mode = kModeCondition;
        condition_stack.top() = true;
      }
    }
    else {
      mode = kModeNextCondition;
    }
  }

  void MachCtlBlk::ConditionElse() {
    if (!condition_stack.empty()) {
      if (condition_stack.top() == true) {
        switch (mode) {
        case kModeCondition:
        case kModeNextCondition:
          mode = kModeNextCondition;
          break;
        case kModeCase:
        case kModeCaseJump:
          mode = kModeCaseJump;
          break;
        default:
          break;
        }
      }
      else {
        management::CreateContainer();
        condition_stack.top() = true;
        switch (mode) {
        case kModeNextCondition:
          mode = kModeCondition;
          break;
        case kModeCaseJump:
          mode = kModeCase;
          break;
	      default:
	        break;
        }
      }
    }
    else {
      SetError("Invaild 'Else'.");
    }
  }

  void MachCtlBlk::LoopHead(bool value) {
    DEBUG_EVENT("(MachCtlBlk)Loop Condition:" + util::MakeBoolean(value));

    if (cycle_nest.empty()) {
      mode_stack.push(mode);
      management::CreateContainer();
    }
    else {
      if (cycle_nest.top() != current - 1) {
        mode_stack.push(mode);
        management::CreateContainer();
      }
    }

    if (value == true) {
      mode = kModeCycle;
      if (cycle_nest.empty() || cycle_nest.top() != current - 1) {
        cycle_nest.push(current - 1);
      }
    }
    else {
      mode = kModeCycleJump;
      if (!cycle_tail.empty()) {
        current = cycle_tail.top();
      }
    }
  }
  
  void MachCtlBlk::End(vector<IR> &storage) {
    if (mode == kModeCondition || mode == kModeNextCondition) {
      condition_stack.pop();
      mode = mode_stack.top();
      mode_stack.pop();
      management::DisposeManager();
    }
    else if (mode == kModeCycle || mode == kModeCycleJump) {
      switch (mode) {
      case kModeCycle:
        if (cycle_tail.empty() || cycle_tail.top() != current - 1) {
          cycle_tail.push(current - 1);
        }
        current = cycle_nest.top();
        management::GetCurrentContainer().clear();
        break;
      case kModeCycleJump:
        if (s_continue) {
          if (cycle_tail.empty() || cycle_tail.top() != current - 1) {
            cycle_tail.push(current - 1);
          }
          current = cycle_nest.top();
          mode = kModeCycle;
          mode_stack.top() = mode;
          s_continue = false;
          management::GetCurrentContainer().clear();
        }
        else {
          if (s_break) s_break = false;
          mode = mode_stack.top();
          mode_stack.pop();
          if (!cycle_nest.empty()) cycle_nest.pop();
          if (!cycle_tail.empty()) cycle_tail.pop();
          management::DisposeManager();
        }
        break;
      default:
        break;
      }
    }
    else if (mode == kModeCase || mode == kModeCaseJump) {
      condition_stack.pop();
      mode = mode_stack.top();
      mode_stack.pop();
      management::DisposeManager();
    }
    else if (mode == kModeClosureCatching) {
      CatchClosure(storage);
      mode = mode_stack.top();
      mode_stack.pop();
    }
  }

  void MachCtlBlk::Continue() {
    while (!mode_stack.empty() && mode != kModeCycle) {
      if (mode == kModeCondition || mode == kModeCase) {
        condition_stack.pop();
      }
      mode = mode_stack.top();
      mode_stack.pop();
    }

    if (mode == kModeCycle) {
      mode = kModeCycleJump;
      s_continue = true;
    }
    else {
      SetError("Illegal 'continue' operation.");
    }
  }

  void MachCtlBlk::Break() {
    while (!mode_stack.empty() && mode != kModeCycle) {
      if (mode == kModeCondition || mode == kModeCase) {
        condition_stack.pop();
      }
      mode = mode_stack.top();
      mode_stack.pop();
    }

    if (mode == kModeCycle) {
      mode = kModeCycleJump;
      s_break = true;
    }
    else {
      SetError("Illegal 'break' operation");
    }
  }

  void MachCtlBlk::Clear() {
    s_continue = false;
    s_break = false;
    last_index = false;
    tail_recursion = false;
    tail_call = false;
    runtime_error = false;
    current = 0;
    def_start = 0;
    mode = kModeNormal;
    error_string.clear();
    while (!cycle_nest.empty()) cycle_nest.pop();
    while (!cycle_tail.empty()) cycle_tail.pop();
    while (!mode_stack.empty()) mode_stack.pop();
    while (!condition_stack.empty()) condition_stack.pop();
    func_string_vec.clear();
    func_string_vec.shrink_to_fit();
    recursion_map.clear();
  }

  void MachCtlBlk::CreateClosureProc(string func_string) {
    func_string_vec = BuildStringVector(func_string);
    fn_idx = current;
    mode_stack.push(mode);
    mode = kModeClosureCatching;
  }

  void MachCtlBlk::CatchClosure(vector<IR> &storage) {
    bool optional = false;
    bool variable = false;
    size_t counter = 0;
    auto &pool = management::GetContainerPool();
    vector<string> params;
    vector<IR> ir;
    
    for (size_t idx = fn_idx + 1; idx < current; idx += 1) {
      ir.push_back(storage[idx]);
    }

    for (size_t i = 1; i < func_string_vec.size(); ++i) {
      if (func_string_vec[i] == kStrOptional) {
        optional = true;
        counter += 1;
        continue;
      }

      if (func_string_vec[i] == kStrVaribale) {
        if (counter == 1) {
          SetError("Variable parameter can be defined only once.");
          break;
        }

        if (i != func_string_vec.size() - 2) {
          SetError("Variable parameter must be last one.");
          break;
        }

        variable = true;
        counter += 1;
        continue;
      }

      if (optional && func_string_vec[i - 1] != kStrOptional) {
        SetError("Optional parameter must be defined after normal parameter.");
        break;
      }

      params.push_back(func_string_vec[i]);
    }

    if (optional && variable) {
      SetError("Variable & Optional parameter can'b be defined at same time.");
      return;
    }

    Interface interface(ir, func_string_vec[0], params, FunctionAgentTunnel);

    if (optional) {
      interface
        .SetMinArgSize(params.size() - counter)
        .SetArgumentMode(kCodeAutoFill);
    }

    if (variable) {
      interface.SetArgumentMode(kCodeAutoSize);
    }

    ObjectMap record;

    auto it = pool.rbegin();

    while (it != pool.rend()) {
      for (auto &unit : it->GetConent()) {
        if (record.find(unit.first) != record.end()) {
          record.insert(NamedObject(unit.first,
            Object(management::type::GetObjectCopy(unit.second), unit.second.GetTypeId())));
        }
      }

      if (it->Find(kStrUserFunc) != nullptr) {
        break;
      }

      ++it;
    }

    if (it != pool.rend()) {
      for (auto &unit : it->GetConent()) {
        if (record.find(unit.first) == record.end()) {
          record.insert(NamedObject(unit.first,
            Object(management::type::GetObjectCopy(unit.second), unit.second.GetTypeId())));
        }
      }
    }
    interface.SetClousureRecord(record);
    pool.back().Add(func_string_vec[0], 
      Object(make_shared<Interface>(interface), kTypeIdFunction));
  }

  Object IRWorker::MakeObject(Argument &arg, bool checking) {
    string domain_type_id = kTypeIdNull;
    Object obj;
    ObjectPointer ptr;

    auto fetching = [&](ArgumentType type, bool is_domain)->bool {
      switch (type) {
      case kArgumentNormal:
        obj.ManageContent(make_shared<string>(arg.data), kTypeIdRawString);
        break;

      case kArgumentObjectPool:
        ptr = management::FindObject(is_domain ? arg.domain.data : arg.data);
        if (ptr != nullptr) {
          if (is_domain) {
            domain_type_id = ptr->GetTypeId();
          }
          else {
            obj.CreateRef(*ptr);
          }
        }
        else {
          obj = is_domain ?
            GetFunctionObject(arg.domain.data, kTypeIdNull) :
            GetFunctionObject(arg.data, domain_type_id);

          if (obj.Get() == nullptr) {
            MakeError("Object is not found." 
              + is_domain ? arg.domain.data : arg.data);
            return false;
          }
        }
        break;

      case kArgumentReturningStack:
        if (!returning_base.empty()) {
          if (is_domain) {
            domain_type_id = returning_base.top().GetTypeId();
          }
          else {
            obj = returning_base.top();
            if (!checking) returning_base.pop();
          }
        }
        else {
          MakeError("Can't get object from stack.");
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

  void IRWorker::Assembling_AutoSize(Interface &interface,
    ArgumentList args, ObjectMap &obj_map) {
    auto ent_params = interface.GetParameters();
    auto va_arg_head = ent_params.back();

    deque<Object> temp;

    while (args.size() >= ent_params.size() - 1 && !args.empty()) {
      temp.emplace_front(MakeObject(args.back()));
      args.pop_back();
    }
    

    shared_ptr<ObjectArray> va_base = make_shared<ObjectArray>();

    if (!temp.empty()) {
      for (auto it = temp.begin(); it != temp.end(); ++it) {
        va_base->emplace_back(*it);
      }
    }

    obj_map.insert(NamedObject(va_arg_head, Object(va_base, kTypeIdArray)));

    temp.clear();
    temp.shrink_to_fit();

    auto it = ent_params.rbegin()++;

    if (!args.empty()) {
      for (; it != ent_params.rend(); ++it) {
        if (args.empty()) break;
        obj_map.insert(NamedObject(*it, MakeObject(args.back())));
        args.pop_back();
      }
    }
  }

  void IRWorker::Assembling_AutoFill(Interface &interface, 
    ArgumentList args, ObjectMap &obj_map) {
    size_t min_size = interface.GetMinArgSize();

    if (args.size() > interface.GetParameters().size()) {
      MakeError("Too many arguments");
      return;
    }

    if (args.size() < min_size) {
      MakeError("Required minimum argument count is" +
        to_string(min_size) +
        ", but provided argument count is" +
        to_string(args.size()));
      return;
    }

    auto ent_params = interface.GetParameters();

    while (args.size() != ent_params.size()) {
      obj_map.insert(NamedObject(ent_params.back(), Object()));
      ent_params.pop_back();
    }

    for (auto it = ent_params.rbegin(); it != ent_params.rend(); ++it) {
      obj_map.insert(NamedObject(*it, MakeObject(args.back())));
      args.pop_back();
    }
  }

  void IRWorker::Assembling(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
    if (args.size() > interface.GetParameters().size()) {
      MakeError("Too many arguments");
    }

    auto ent_params = interface.GetParameters();
    bool state = true;
    
    for (auto it = ent_params.rbegin(); it != ent_params.rend(); it++) {
      if (args.empty()) {
        state = false;
        break;
      }
      obj_map.insert(NamedObject(*it, MakeObject(args.back())));
      args.pop_back();
    }

    if (!state) {
      MakeError("Required argument count is " +
        to_string(ent_params.size()) +
        ", but provided argument count is " +
        to_string(args.size()) + ".");
    }
  }

  void IRWorker::GenerateArgs(StateCode code, Interface &interface, ArgumentList args, ObjectMap & obj_map) {
    switch (code) {
    case kCodeAutoSize:
      Assembling_AutoSize(interface, args, obj_map);
      break;
    case kCodeAutoFill:
      Assembling_AutoFill(interface, args, obj_map);
      break;
    default:
      Assembling(interface, args, obj_map);
      break;
    }
  }

  void IRWorker::Reset() {
    error = false;
    error_string.clear();
    tail_recursion = false;
    deliver = false;
    msg.Clear();
    while (!returning_base.empty()) returning_base.pop();
  }

  void IRWorker::Bind(ArgumentList args) {
    auto src = MakeObject(args[1]);
    auto dest = MakeObject(args[0]);

    if (dest.IsRef()) {
      CopyObject(dest, src);
    }
    else {
      string id = dest.Cast<string>();

      DEBUG_EVENT("(IRWorker)Binding new object:" + id);

      if (util::GetTokenType(id, true) == kTokenTypeGeneric) {
        ObjectPointer real_dest = management::FindObject(id);

        if (real_dest != nullptr) {
          CopyObject(*real_dest, src);
        }
        else {
          Object obj(management::type::GetObjectCopy(src), src.GetTypeId());
          ObjectPointer result_ptr = management::CreateObject(id, obj);
          if (result_ptr == nullptr) {
            MakeError("Object cration is failed.");
          }
        }
      }
      else {
        MakeError("Invalid object id - " + id);
      }
    }
  }

  void IRWorker::ExpList(ArgumentList args) {
    if (!args.empty()) {
      auto obj = MakeObject(args.back());
      returning_base.push(obj);
    }
  }

  void IRWorker::InitArray(ArgumentList args) {
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

    if (!args.empty()) {
      for (auto it = args.begin(); it != args.end(); ++it) {
        base->emplace_back(MakeObject(*it));
      }
    }

    Object obj(base, kTypeIdArray);
    obj.SetConstructorFlag();
    returning_base.push(obj);
  }

  void IRWorker::ReturnOperator(ArgumentList args) {
    auto &container = management::GetCurrentContainer();
    if (args.size() > 1) {
      shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

      for (auto it = args.begin(); it != args.end(); ++it) {
        base->emplace_back(MakeObject(*it));
      }

      Object obj(base, kTypeIdArray);
      container.Add(kStrRetValue, Object(obj.Get(), obj.GetTypeId()));
    }
    else if (args.size() == 1) {
      auto obj = MakeObject(args[0]);

      container.Add(kStrRetValue, Object(obj.Get(), obj.GetTypeId()));
    }

    MakeCode(kCodeReturn);
  }

  void IRWorker::GetTypeId(ArgumentList args) {
    if (args.size() > 1) {
      shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

      for (auto it = args.begin(); it != args.end(); ++it) {
        auto obj = MakeObject(*it);
        base->emplace_back(Object(obj.GetTypeId()));
      }

      Object ret_obj(base, kTypeIdArray);
      ret_obj.SetConstructorFlag();
      returning_base.push(ret_obj);
    }
    else if (args.size() == 1) {
      auto obj = MakeObject(args[0]);
      returning_base.push(Object(obj.GetTypeId()));
    }
    else {
      MakeError("Empty argument list");
    }
  }

  void IRWorker::GetMethods(ArgumentList args) {
    if (!args.empty()) {
      Object obj = MakeObject(args[0]);
      vector<string> methods = management::type::GetMethods(obj.GetTypeId());
      shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

      for (const auto &unit : methods) {
        base->emplace_back(Object(make_shared<string>(unit), kTypeIdString));
      }

      Object ret_obj(base, kTypeIdArray);
      ret_obj.SetConstructorFlag();
      returning_base.push(ret_obj);
    }
    else {
      MakeError("Empty argument list.");
    }
  }

  void IRWorker::Exist(ArgumentList args) {
    bool result = true;

    if (args.size() == 2) {
      Object obj = MakeObject(args[0]);
      Object str_obj = MakeObject(args[1]);
      string target = ParseRawString(str_obj.Cast<string>());
      vector<string> methods = management::type::GetMethods(obj.GetTypeId());
      Object ret_obj(util::MakeBoolean(find_in_vector<string>(target, methods)));

      returning_base.push(ret_obj);
    }
    else if (args.size() > 2) {
      MakeError("Too many arguments");
    }
    else if (args.size() < 2) {
      MakeError("Too few arguments.");
    }
  }

  void IRWorker::Fn(ArgumentList args) {
    if (!args.empty()) {
      vector<string> func_string_vec;

      for (auto it = args.begin(); it != args.end(); ++it) {
        func_string_vec.emplace_back(it->data);
      }

      string def_head_string = util::CombineStringVector(func_string_vec);
      MakeMsg(Message(kCodeFunctionCatching, def_head_string));
    }
    else {
      MakeError("Empty argument list.");
    }
  }

  void IRWorker::Case(ArgumentList args) {
    bool result = true;

    if (!args.empty()) {
      Object obj = MakeObject(args[0]);

      if (!IsStringObject(obj)) {
        MakeError("Case-when is not supported for non-string object.");
      }
      else {
        auto copy = management::type::GetObjectCopy(obj);
        Object base(copy, obj.GetTypeId());
        management::CreateObject("__case", base);
        MakeMsg(Message().SetCode(kCodeCase));
      }
    }
    else {
      MakeError("Empty argument list.");
    }
  }

  // TODO: add compare method support
  void IRWorker::When(ArgumentList args) {
    if (!args.empty()) {
      ObjectPointer case_head = management::FindObject("__case");
      string case_content = case_head->Cast<string>();
      bool state = true, found = false;

      for (auto it = args.begin(); it != args.end(); ++it) {
        auto obj = MakeObject(*it);

        if (!IsStringObject(obj)) {
          state = false;
          break;
        }

        if (case_content == obj.Cast<string>()) {
          found = true;
          break;
        }
      }

      if (state) {
        MakeMsg(Message(kCodeWhen, found ? kStrTrue : kStrFalse));
      }
      else {
        MakeError("Case-when is not supported for non-string object.");
      }
    }
    else {
      MakeError("Empty argument list.");
    }
  }

  void IRWorker::DomainAssert(ArgumentList args, bool returning, bool no_feeding) {
    Object obj = MakeObject(args[0], no_feeding);
    Object id_obj = MakeObject(args[1]);
    string id = id_obj.Cast<string>();
    vector<string> methods = management::type::GetMethods(obj.GetTypeId());
    bool result = find_in_vector<string>(id, methods);

    if (!result) {
      MakeError("Method/Member is not found. - " + id);
      return;
    }

    if (returning) {
      auto interface = management::FindInterface(id, obj.GetTypeId());

      if (!interface.Good()) {
        MakeError("Method/Member is not found. - " + id);
        return;
      }

      Object ent_obj(make_shared<Interface>(interface), kTypeIdFunction);
      returning_base.push(ent_obj);
    }
  }

  void IRWorker::ConditionAndLoop(ArgumentList args, StateCode code) {
    if (args.size() == 1) {
      Object obj = MakeObject(args[0]);

      if (obj.GetTypeId() != kTypeIdRawString && obj.GetTypeId() != kTypeIdNull) {
        MakeMsg(Message(code, kStrTrue));
      }
      else {
        string state_str = obj.Cast<string>();

        if (compare(state_str, { kStrTrue,kStrFalse })) {
          MakeMsg(Message(code, state_str));
        }
        else {
          auto type = util::GetTokenType(state_str, true);

          bool state = false;
          switch (type) {
          case kTokenTypeInt:
            state = (stol(state_str) != 0);
            break;
          case kTokenTypeFloat:
            state = (stod(state_str) != 0.0);
            break;
          case kTokenTypeString:
            state = (ParseRawString(state_str).size() > 0);
            break;
          default:
            break;
          }

          MakeMsg(Message(code, state ? kStrTrue : kStrFalse));
        }
      }
    }
    else if (args.empty()) {
      MakeError("Empty argument list.");
    }
    else {
      MakeError("Too many arguments.");
    }
  }

  IRMaker::IRMaker(const char *path) : health(true) {
    DEBUG_EVENT("IR Generating start");

    std::wifstream stream(path);
    wstring buf;
    string temp;
    vector<string> script_buf;
    vector<StringUnit> string_units;
    Analyzer analyzer;
    Message msg;
    size_t error_counter = 0;

    if (!stream.good()) {
      health = false;
      return;
    }

    while (!stream.eof()) {
      std::getline(stream, buf);
      temp = ws2s(buf);
      if (!temp.empty() && temp.back() == '\0') temp.pop_back();
      script_buf.emplace_back(temp);
    }

    stream.close();

    string_units = MultilineProcessing(script_buf);
    script_buf.clear();
    script_buf.shrink_to_fit();

    for (auto it = string_units.begin();it != string_units.end(); it++) {
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

      output.emplace_back(IR(
        analyzer.GetOutput(),
        analyzer.get_index(),
        analyzer.GetMainToken()
      ));

      analyzer.Clear();
    }

    DEBUG_EVENT("IR Generating complete. IR count is " + to_string(output.size()));
  }

  void Module::ResetContainer(string func_id) {
    Object *id_obj = management::GetCurrentContainer().Find(kStrUserFunc);

    while (id_obj == nullptr) {
      management::DisposeManager();
      id_obj = management::GetCurrentContainer().Find(kStrUserFunc);
      if (id_obj != nullptr) {
        string value = id_obj->Cast<string>();
        if (value == func_id) break;
      }
    }
  }

  bool Module::MakeFunction(size_t start, size_t end, vector<string> &func_string_vec, 
    int &event_code) {
    if (start > end) return false;
    
    bool result = true;
    bool optional = false;
    bool variable = false;
    
    string id = func_string_vec[0];
    vector<string> params;
    vector<IR> proc;
    size_t counter = 0;

    for (size_t i = 1; i < func_string_vec.size(); ++i) {
      if (func_string_vec[i] == kStrOptional) {
        optional = true;
        counter += 1;
        continue;
      }

      if (func_string_vec[i] == kStrVaribale) {
        if (counter == 1) {
          event_code = 1;
          result = false;
          break;
        }

        if (i != func_string_vec.size() - 2) {
          result = false;
          event_code = 2;
          break;
        }

        variable = true;
        counter += 1;
        continue;
      }

      if (optional && func_string_vec[i - 1] != kStrOptional) {
        event_code = 3;
        result = false;
        break;
      }

      params.push_back(func_string_vec[i]);
    }

    if (optional && variable) {
      event_code = 4;
      result = false;
      return result;
    }

    for (size_t j = start; j <= end; ++j) {
      proc.push_back(storage_[j]);
    }

    Interface interface(proc, id, params, FunctionAgentTunnel);

    if (optional) { 
      interface
        .SetMinArgSize(params.size() - counter)
        .SetArgumentMode(kCodeAutoFill);
    }

    if (variable) {
      interface.SetArgumentMode(kCodeAutoSize);
    }

    if (result) management::CreateNewInterface(interface);

    return result;
  }

  Message Module::IRProcessing(IR &ir, string name, MachCtlBlk *blk) {
    bool preprocessing = (blk == nullptr);
    IRWorker *worker = new IRWorker();
    string type_id, value;
    Message msg;
    ObjectMap obj_map;
    Interface interface;
    vector<Command> &action_base = ir.GetContains();
    size_t size = action_base.size();

    for (size_t idx = 0; idx < size; idx += 1) {
      obj_map.clear();

      auto &request = action_base[idx].first;
      auto &args = action_base[idx].second;

      if (request.type == kRequestCommand && !util::IsOperator(request.head_command)) {
        GenericRequests(worker, request, args);

        if (worker->deliver) {
          msg = worker->GetMsg();
          worker->msg.Clear();
        }

        if (worker->error) break;

        continue;
      }

      if (request.type == kRequestCommand) {
        interface = management::GetGenericInterface(request.head_command);
      }

      if (request.type == kRequestInterface) {
        bool state = true;
        //Tail Recursion Detection
        if (!preprocessing && request.head_interface == name) {
          if (idx == size - 1) {
            worker->tail_recursion = blk->last_index && name != "";
          }
          else if (idx = size - 2) {
            worker->tail_recursion = action_base.back().first.head_command == kTokenReturn;
          }
        }

        //Find interface
        if (request.domain.type != kArgumentNull) {
          Object domain_obj = worker->MakeObject(request.domain, true);
          type_id = domain_obj.GetTypeId();
          interface = management::FindInterface(request.head_interface, type_id);
          obj_map.insert(NamedObject(kStrObject, domain_obj));
        }
        else {
          Object *func_obj = management::FindObject(request.head_interface);
          if (func_obj != nullptr) {
            if (func_obj->GetTypeId() == kTypeIdFunction) {
              interface = func_obj->Cast<Interface>();
            }
            else {
              worker->MakeError(request.head_interface + " is not a function object.");
              state = false;
            }
          }
          else {
            interface = management::FindInterface(request.head_interface);
          }
        }

        if (!state) break;

        if (!interface.Good()) {
          worker->MakeError("Function is not found - " + request.head_interface);
          break;
        }
      }

      worker->GenerateArgs(interface.GetArgumentMode(), interface, args, obj_map);

      if (worker->error) {
        break;
      }

      if (worker->tail_recursion) {
        blk->recursion_map = obj_map;
        break;
      }

      msg = interface.Start(obj_map);

      if (msg.GetLevel() == kStateError) break;

      Object object = msg.GetCode() == kCodeObject ?
        msg.GetObj() : Object();

      worker->returning_base.push(object);
    }

    if (worker->error) {
      msg = Message(kCodeIllegalSymbol, worker->error_string, kStateError);
    }

    if (!preprocessing && worker->tail_recursion)
      blk->tail_recursion = worker->tail_recursion;

    obj_map.clear();
    while(!worker->returning_base.empty()) worker->returning_base.pop();

    delete worker;

    return msg;
  }

  Message Module::PreProcessing() {
    IR *ir = nullptr;
    GenericToken token;
    Message result;
    bool catching = false;
    map<size_t, size_t> catched_block;
    size_t nest_counter = 0;
    long fn_idx = -1;
    vector<string> func_string_vec;

    for (size_t idx = 0; idx < storage_.size(); idx += 1) {
      ir = &storage_[idx];
      token = util::GetGenericToken(ir->GetMainToken().first);

      if (token == kTokenFn) {
        if (catching) {
          //skip closure block
          nest_counter += 1;
          continue;
        }
        else {
          result = IRProcessing(*ir, "", nullptr);
          func_string_vec = BuildStringVector(result.GetDetail());
          fn_idx = static_cast<long>(idx);
          //enter catching process
          catching = true;
          continue;
        }
      }
      else if (token == kTokenEnd) {
        if (nest_counter > 0) {
          nest_counter -= 1;
          continue;
        }
        else {
          catched_block.insert(std::make_pair(fn_idx, idx));
          int code;
          if (!MakeFunction(fn_idx + 1, idx - 1, func_string_vec, code)) {
            switch (code) {
            case 1:result = INVALID_PARAM_MSG("Variable parameter can be defined only once."); break;
            case 2:result = INVALID_PARAM_MSG("Variable parameter must be last one."); break;
            case 3:result = INVALID_PARAM_MSG("Optional parameter must be defined after normal parameter."); break;
            case 4:result = INVALID_PARAM_MSG("Variable & Optional parameter can'b be defined at same time."); break;
            default:break;
            }
          }
          func_string_vec.clear();
          fn_idx = -1;
          //quit catching process
          catching = false;
        }
      }
      else {
        if (find_in_vector(token, nest_flag_collection)) {
          nest_counter += 1;
        }
      }
    }

    if (catching) {
      result = BAD_EXP_MSG("End token is not found(Fn index:)" + to_string(fn_idx));
      return result;
    }

    vector<IR> not_catched;

    for (size_t idx = 0; idx < storage_.size(); idx += 1) {
      auto it = catched_block.find(idx);
      if (it != catched_block.end()) {
        idx = it->second;
        continue;
      }

      not_catched.emplace_back(storage_[idx]);
    }

    storage_.swap(not_catched);

    return result;
  }

  void Module::TailRecursionActions(MachCtlBlk *blk, string &name) {
    Object obj = *management::FindObject(kStrUserFunc);
    auto &base = management::GetCurrentContainer();
    base.clear();
    base.Add(kStrUserFunc, obj);
    for (auto &unit : blk->recursion_map) {
      base.Add(unit.first, unit.second);
    }
    blk->recursion_map.clear();
    blk->Clear();
    ResetContainer(name);
  }


  void Module::CallMachineFunction(StateCode code, string detail, MachCtlBlk *blk) {
    switch (code) {
    case kCodeFunctionCatching:
      blk->CreateClosureProc(detail);
      break;
    case kCodeContinue:
      blk->Continue();
      break;
    case kCodeBreak:
      blk->Break();
      break;
    case kCodeConditionIf:
      blk->ConditionIf(GetBooleanValue(detail));
      break;
    case kCodeConditionElif:
      blk->ConditionElif(GetBooleanValue(detail));
      break;
    case kCodeConditionElse:
      blk->ConditionElse();
      break;
    case kCodeCase:
      blk->Case();
      break;
    case kCodeWhen:
      blk->When(GetBooleanValue(detail));
      break;
    case kCodeWhile:
      blk->LoopHead(GetBooleanValue(detail));
      break;
    case kCodeEnd:
      blk->End(storage_);
      break;
    default:break;
    }
  }

  void Module::GenericRequests(IRWorker *worker, Request &request, ArgumentList &args) {
    auto &token = request.head_command;
    bool result = true;

    //DEBUG_EVENT("(Request)Command Code:" + to_string(token));

    switch (token) {
    case kTokenBind:
      worker->Bind(args);
      break;

    case kTokenExpList:
      worker->ExpList(args);
      break;

    case kTokenInitialArray:
      worker->InitArray(args);
      break;

    case kTokenReturn:
      worker->ReturnOperator(args);
      break;

    case kTokenTypeId:
      worker->GetTypeId(args);
      break;

    case kTokenDir:
      worker->GetMethods(args);
      break;

    case kTokenExist:
      worker->Exist(args);
      break;

    case kTokenFn:
      worker->Fn(args);
      break;

    case kTokenCase:
      worker->Case(args);
      break;

    case kTokenWhen:
      worker->When(args);
      break;

    case kTokenAssert:
    case kTokenAssertR:
      worker->DomainAssert(args, 
        token == kTokenAssertR, request.option.no_feeding);
      break;

    case kTokenQuit:
      worker->MakeCode(kCodeQuit);
      break;

    case kTokenEnd:
      worker->MakeCode(kCodeEnd);
      break;

    case kTokenContinue:
      worker->MakeCode(kCodeContinue);
      break;

    case kTokenBreak:
      worker->MakeCode(kCodeBreak);
      break;

    case kTokenElse:
      worker->MakeCode(kCodeConditionElse);
      break;

    case kTokenIf:
      worker->ConditionAndLoop(args, kCodeConditionIf);
      break;

    case kTokenElif:
      worker->ConditionAndLoop(args, kCodeConditionElif);
      break;

    case kTokenWhile:
      worker->ConditionAndLoop(args, kCodeWhile);
      break;

    default:
      break;
    }
  }

  bool Module::SkippingWithCondition(MachCtlBlk *blk,
    std::initializer_list<GenericToken> terminators) {
    GenericToken token;
    size_t nest_counter = 0;
    bool flag = false;
    
    while (blk->current < storage_.size()) {
      token = util::GetGenericToken(storage_[blk->current].GetMainToken().first);

      if (find_in_vector(token, nest_flag_collection)) {
        nest_counter += 1;
        blk->current += 1;
        continue;
      }

      if (compare(token, terminators)) {
        if (nest_counter == 0) {
          flag = true;
          break;
        }
        else {
          blk->current += 1;
          continue;
        }
      }

      if (token == kTokenEnd) {
        if (nest_counter != 0) {
          nest_counter -= 1;
          blk->current += 1;
          continue;
        }
        else {
          flag = true;
          break;
        }
      }

      blk->current += 1;
    }

    return flag;
  }

  bool Module::Skipping(MachCtlBlk *blk) {
    bool flag = false;
    size_t nest_counter = 0;
    GenericToken token;

    while (blk->current < storage_.size()) {
      token = util::GetGenericToken(storage_[blk->current].GetMainToken().first);

      if (find_in_vector(token, nest_flag_collection)) {
        nest_counter += 1;
        blk->current += 1;
        continue;
      }

      if (token == kTokenEnd) {
        if (nest_counter != 0) {
          nest_counter -= 1;
          blk->current += 1;
          continue;
        }
        else {
          flag = true;
          break;
        }
      }

      blk->current += 1;
    }

    return flag;
  }

  bool Module::SkippingStrategy(MachCtlBlk *blk) {
    bool result;

    switch (blk->mode) {
    case kModeNextCondition:
      result = SkippingWithCondition(blk, { kTokenElif, kTokenElse });
      break;
    case kModeCaseJump:
      result = SkippingWithCondition(blk, { kTokenWhen,kTokenElse });
      break;
    case kModeCycleJump:
    case kModeClosureCatching:
      result = Skipping(blk);
      break;
    default:
      break;
    }

    return result;
  }

  Module::Module(IRMaker &maker, bool is_main) :
    is_main_(is_main) {

    Message msg;

    if (maker.health) {
      storage_ = maker.output;
      //these need to modify for module feature.
      if (is_main) {
        msg = PreProcessing();
        if (msg.GetLevel() == kStateError) {
          trace::AddEvent(msg);
          storage_.clear();
          storage_.shrink_to_fit();
        }
      }
    }
    else {
      trace::AddEvent(Message(kCodeBadStream, "Invalid script.", kStateError));
    }
  }

  Message Module::Run(bool create_container, string name) {
    if (storage_.empty()) return Message();

    StateLevel level = kStateNormal;
    StateCode code = kCodeSuccess;
    IR *ir = nullptr;
    MachCtlBlk *blk = new MachCtlBlk();
    string detail;
    Message result;

    if (create_container) management::CreateContainer();

    if (storage_.empty()) return result;

    while (blk->current < storage_.size()) {
      blk->last_index = (blk->current == storage_.size() - 1);
      
      if (NeedSkipping(blk->mode)) {
        result.Clear();
        if (!SkippingStrategy(blk)) {
          blk->SetError("End token is not found.");
          break;
        }
      }

      ir = &storage_[blk->current];
      result = IRProcessing(*ir, name, blk);
      level = result.GetLevel();
      code = result.GetCode();
      detail = result.GetDetail();

      if (level != kStateNormal) {
        trace::AddEvent(result.SetIndex(storage_[blk->current].GetIndex()));
        if (level == kStateError) {
          break;
        }
      }

      if (blk->tail_recursion) {
        TailRecursionActions(blk, name);
        continue;
      }

      if (code == kCodeReturn) {
        break;
      }

      CallMachineFunction(code, detail, blk);

      if (blk->runtime_error) break;

      blk->current += 1;
    }

    if (blk->runtime_error) {
      trace::AddEvent(Message(kCodeBadExpression, blk->error_string, kStateError)
        .SetIndex(blk->current));
    }

    if (create_container) management::DisposeManager();

    delete blk;
    return result;
  }

  Message Module::RunAsFunction(ObjectMap &p) {
    Message msg;
    auto &base = management::CreateContainer();
    string func_id = p.Cast<string>(kStrUserFunc);

    for (auto &unit : p) {
      base.Add(unit.first, unit.second);
    }

    msg = Run(false, func_id);

    if (msg.GetLevel() == kStateError) {
      msg = Message(kCodeIllegalCall, "Error occured in " + func_id, kStateError);
    }

    if (msg.GetLevel() == kStateNormal) {
      msg = Message();
      auto &currentBase = management::GetCurrentContainer();
      Object *ret = currentBase.Find(kStrRetValue);
      if (ret != nullptr) {
        Object obj;
        obj.CloneFrom(*ret);
        msg.SetObject(obj);
      }
    }

    Object *id_obj = management::GetCurrentContainer().Find(kStrUserFunc);

    ResetContainer(func_id);

    management::DisposeManager();
    return msg;
  }

  void Module::Reset(MachCtlBlk *blk) {
    while (!blk->cycle_nest.empty()) blk->cycle_nest.pop();
    while (!blk->cycle_tail.empty()) blk->cycle_tail.pop();
    while (!blk->mode_stack.empty()) blk->mode_stack.pop();
    while (!blk->condition_stack.empty()) blk->condition_stack.pop();
    delete blk;
  }
}

