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

  bool MachCtlBlk::ConditionElse() {
    bool result = true;
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
      result = false;
    }

    return result;
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
      runtime_error = true;
      error_string = "Illegal 'continue' operation.";
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
      runtime_error = true;
      error_string = "Illegal 'break' operation";
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
    auto &pool = management::GetContainerPool();
    vector<string> params;
    vector<IR> ir;

    for (size_t idx = fn_idx + 1; idx < current; idx += 1) {
      ir.push_back(storage[idx]);
    }

    for (size_t idx = 1; idx < func_string_vec.size(); idx += 1) {
      params.push_back(func_string_vec[idx]);
    }

    Interface interface(ir, func_string_vec[0], params, FunctionAgentTunnel);
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
    Object obj, obj_domain;
    ObjectPointer ptr;
    bool state = true;

    switch (arg.domain.type) {
    case kArgumentReturningStack:
      if (!returning_base.empty()) {
        obj_domain = returning_base.top();
      }
      else {
        error_returning = true;
        error_string = "Returning base error.";
        state = false;
      }
      break;
    case kArgumentObjectPool:
      ptr = management::FindObject(arg.domain.data);
      if (ptr != nullptr) {
        obj_domain.CreateRef(*ptr);
      }
      else {
        obj_domain = GetFunctionObject(arg.domain.data, kTypeIdNull);
        if (obj_domain.Get() == nullptr) {
          error_obj_checking = true;
          error_string = "(1)Object is not found - " + arg.data;
          state = false;
        }
      }
      break;
    default:
      break;
    }

    if (!state) return Object();

    switch (arg.type) {
    case kArgumentNormal:
      obj.ManageContent(make_shared<string>(arg.data), kTypeIdRawString);
      break;
    case kArgumentObjectPool:
      ptr = management::FindObject(arg.data);
      if (ptr != nullptr) {
        obj.CreateRef(*ptr);
      }
      else {
        obj = GetFunctionObject(arg.data, obj_domain.GetTypeId());
        if (obj.GetTypeId() == kTypeIdNull) {
          error_obj_checking = true;
          error_string = "(2)Object is not found - " + arg.data;
        }
      }
      break;
    case kArgumentReturningStack:
      if (!returning_base.empty()) {
        obj = returning_base.top();
        if (!checking)
          returning_base.pop();
      }
      else {
        error_returning = true;
        error_string = "Returning base error.";
      }
      break;
    default:
      break;
    }

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
    if (args.size() > interface.GetParameters().size()) {
      error_assembling = true;
      error_string = "Too many arguments.";
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
      error_assembling = true;
      error_string = "Too many arguments.";
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
      error_assembling = true;
      error_string = "Required argument count is " +
        to_string(ent_params.size()) +
        ", but provided argument count is " +
        to_string(args.size()) + ".";
    }
  }

  void IRWorker::Reset() {
    error_string.clear();
    error_returning = false;
    error_obj_checking = false;
    tail_recursion = false;
    error_assembling = false;
    deliver = false;
    msg = Message();
    while (!returning_base.empty()) returning_base.pop();
  }

  bool IRWorker::Bind(ArgumentList args) {
    bool result = true;
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
            error_string = "Object cration is failed.";
            result = false;
          }
        }
      }
      else {
        error_string = "Invalid object id - " + id;
        result = false;
      }
    }

    return result;
  }

  bool IRWorker::ExpList(ArgumentList args) {
    if (!args.empty()) {
      auto obj = MakeObject(args.back());
      returning_base.push(obj);
    }

    return true;
  }

  bool IRWorker::InitArray(ArgumentList args) {
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

    if (!args.empty()) {
      for (auto it = args.begin(); it != args.end(); ++it) {
        base->emplace_back(MakeObject(*it));
      }
    }

    Object obj(base, kTypeIdArray);
    obj.SetConstructorFlag();
    returning_base.push(obj);
  
    return true;
  }

  bool IRWorker::ReturnOperator(ArgumentList args) {
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
    msg = Message(kCodeReturn, "");
    deliver = true;

    return true;
  }

  bool IRWorker::GetTypeId(ArgumentList args) {
    bool result = true;

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
      error_string = "Empty argument list.";
      result = false;
    }

    return result;
  }

  bool IRWorker::GetMethods(ArgumentList args) {
    bool result = true;

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
      error_string = "Empty argument list.";
      result = false;
    }

    return result;
  }

  bool IRWorker::Exist(ArgumentList args) {
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
      error_string = "Too many arguments.";
      result = false;
    }
    else if (args.size() < 2) {
      error_string = "Too few arguments.";
      result = false;
    }

    return result;
  }

  bool IRWorker::Fn(ArgumentList args) {
    bool result = true;

    if (!args.empty()) {
      vector<string> func_string_vec;

      for (auto it = args.begin(); it != args.end(); ++it) {
        func_string_vec.emplace_back(it->data);
      }

      string def_head_string = util::CombineStringVector(func_string_vec);
      deliver = true;
      msg = Message(kCodeFunctionCatching, def_head_string);
    }
    else {
      error_string = "Empty argument list.";
      result = false;
    }

    return result;
  }

  bool IRWorker::Case(ArgumentList args) {
    bool result = true;

    if (!args.empty()) {
      Object obj = MakeObject(args[0]);

      if (!IsStringObject(obj)) {
        result = false;
        error_string = "Case-when is not supported for this object.";
      }
      else {
        auto copy = management::type::GetObjectCopy(obj);
        Object base(copy, obj.GetTypeId());
        management::CreateObject("__case", base);
        deliver = true;
        msg = Message().SetCode(kCodeCase);
      }
    }
    else {
      result = false;
      error_string = "Empty argument list.";
    }

    return result;
  }

  // TODO: add compare method support
  bool IRWorker::When(ArgumentList args) {
    bool result = true;

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
        deliver = true;
        msg = found ?
          Message(kCodeWhen, kStrTrue) :
          Message(kCodeWhen, kStrFalse);
      }
      else {
        error_string = "Case-when is not supported for non-string object.";
        result = false;
      }
    }
    else {
      error_string = "Empty argument list.";
      result = false;
    }

    return result;
  }

  bool IRWorker::DomainAssert(ArgumentList args, bool returning, bool no_feeding) {
    Object obj = MakeObject(args[0], no_feeding);
    Object id_obj = MakeObject(args[1]);
    string id = id_obj.Cast<string>();
    vector<string> methods = management::type::GetMethods(obj.GetTypeId());
    bool result = find_in_vector<string>(id, methods);

    if (!result) {
      error_string = "Method/Member is not found. - " + id;
      return result;
    }

    if (returning) {
      auto interface = management::FindInterface(id, obj.GetTypeId());

      if (!interface.Good()) {
        result = false;
        error_string = "Method/Member is not found. - " + id;
        return result;
      }

      Object ent_obj(make_shared<Interface>(interface), kTypeIdFunction);
      returning_base.push(ent_obj);
    }

    return result;
  }

  bool IRWorker::ConditionAndLoop(ArgumentList args, StateCode code) {
    bool result = true;
    if (args.size() == 1) {
      Object obj = MakeObject(args[0]);

      deliver = true;

      if (obj.GetTypeId() != kTypeIdRawString && obj.GetTypeId() != kTypeIdNull) {
        msg = Message(code, kStrTrue);
      }
      else {
        string state_str = obj.Cast<string>();

        if (compare(state_str, { kStrTrue,kStrFalse })) {
          msg = Message(code, state_str);
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

          msg = Message(code, state ? kStrTrue : kStrFalse);
        }
      }
    }
    else if (args.empty()) {
      error_string = "Too few arguments.";
      result = false;
    }
    else {
      error_string = "Too many arguments.";
      result = false;
    }

    return result;
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
      if (!health) break;

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

  void Module::MakeFunction(size_t start, size_t end, vector<string> &func_string_vec) {
    if (start > end) return;
    string id = func_string_vec[0];
    vector<string> params;
    vector<IR> proc;

    for (size_t i = 1; i < func_string_vec.size(); ++i) {
      params.push_back(func_string_vec[i]);
    }
    for (size_t j = start; j <= end; ++j) {
      proc.push_back(storage_[j]);
    }

    management::CreateNewInterface(
      Interface(proc, id, params, FunctionAgentTunnel)
    );
  }

  bool Module::IsBlankStr(string target) {
    if (target.empty() || target.size() == 0) return true;
    for (const auto unit : target) {
      if (!compare(unit, { '\n',' ','\t','\r' })) {
        return false;
      }
    }
    return true;
  }

  Message Module::IRProcessing(IR &ir, string name, MachCtlBlk *blk) {
    Message msg;
    ObjectMap obj_map;
    vector<Command> &action_base = ir.GetContains();
    bool preprocessing = (blk == nullptr),
      is_operator_token = false;

    IRWorker *worker = new IRWorker();

    for (size_t idx = 0; idx < action_base.size(); idx += 1) {
      obj_map.clear();

      auto &request = action_base.at(idx).first;
      auto &args = action_base.at(idx).second;

      if (request.type == kRequestCommand && CheckGenericRequests(request.head_command)) {
        (request.head_interface == name && idx == action_base.size() - 2
          && action_base.back().first.head_command == kTokenReturn) ?
          worker->tail_recursion = true :
          worker->tail_recursion = false;

        bool state = GenericRequests(worker, request, args);

        if (worker->deliver) {
          msg = worker->msg;
          worker->deliver = false;
          worker->msg = Message();
        }

        if (!state) {
          break;
        }
      }
      else if (request.type == kRequestInterface || (request.type == kRequestCommand && !CheckGenericRequests(request.head_command))) {
        bool state = true;
        Interface interface;
        string type_id, value;
        is_operator_token = util::IsOperatorToken(request.head_command);

        if (!preprocessing && worker->tail_recursion) {
          worker->tail_recursion = (
            request.head_interface == name
            && idx == action_base.size() - 1
            && blk->last_index
            && !compare(name, { "", "__null__" })
            );
        }

        switch (request.type) {
        case kRequestCommand:
          //DEBUG_EVENT("(Request)Command code:" + to_string(request.head_command));
          interface = management::GetGenericInterface(request.head_command);
          break;
        case kRequestInterface:
          //DEBUG_EVENT("(Request)Interface id:" + request.head_interface);
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
                worker->error_obj_checking = true;
                worker->error_string = request.head_interface + " is not a function object.";
                state = false;
              }
            }
            else {
              interface = management::FindInterface(request.head_interface);
            }
          }
          break;
        default:
          break;
        }

        if (!state) break;

        if (!interface.Good()) {
          switch (request.type) {
          case kRequestInterface:
            msg = Message(kCodeIllegalCall,
              "Function is not found - " + request.head_interface,
              kStateError);
            break;
          case kRequestCommand:
            msg = Message(kCodeIllegalCall,
              "IR Framework Panic, please check interpreter version or contact author.",
              kStateError);
            break;
          default:break;
          }

          break;
        }

        switch (interface.GetArgumentMode()) {
        case kCodeAutoSize:
          worker->Assembling_AutoSize(interface, args, obj_map);
          break;
        case kCodeAutoFill:
          worker->Assembling_AutoFill(interface, args, obj_map);
          break;
        default:
          worker->Assembling(interface, args, obj_map);
          break;
        }

        if (worker->error_returning 
          || worker->error_obj_checking 
          || worker->error_assembling) {
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
    }

    if (worker->error_returning 
      || worker->error_obj_checking 
      || worker->error_assembling) {
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
      if (!health_) break;

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
          MakeFunction(fn_idx + 1, idx - 1, func_string_vec);
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
      result = Message(kCodeIllegalCall,
        "End token is not found(Fn index:)" + to_string(fn_idx), kStateError);
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
      health_ = blk->ConditionElse();
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

  bool Module::GenericRequests(IRWorker *worker, Request &request, ArgumentList &args) {
    auto &token = request.head_command;
    bool result = true;

    //DEBUG_EVENT("(Request)Command Code:" + to_string(token));

    switch (token) {
    case kTokenBind:
      result = worker->Bind(args);
      break;

    case kTokenExpList:
      result = worker->ExpList(args);
      break;

    case kTokenInitialArray:
      result = worker->InitArray(args);
      break;

    case kTokenReturn:
      result = worker->ReturnOperator(args);
      break;

    case kTokenTypeId:
      result = worker->GetTypeId(args);
      break;

    case kTokenDir:
      result = worker->GetMethods(args);
      break;

    case kTokenExist:
      result = worker->Exist(args);
      break;

    case kTokenFn:
      result = worker->Fn(args);
      break;

    case kTokenCase:
      result = worker->Case(args);
      break;

    case kTokenWhen:
      result = worker->When(args);
      break;

    case kTokenAssert:
    case kTokenAssertR:
      result = worker->DomainAssert(args, 
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
      result = worker->ConditionAndLoop(args, kCodeConditionIf);
      break;

    case kTokenElif:
      result = worker->ConditionAndLoop(args, kCodeConditionElif);
      break;

    case kTokenWhile:
      result = worker->ConditionAndLoop(args, kCodeWhile);
      break;

    default:
      break;
    }

    return result;
  }

  bool Module::CheckGenericRequests(GenericToken token) {
    bool result = false;

    switch (token) {
    case kTokenBind:
    case kTokenExpList:
    case kTokenInitialArray:
    case kTokenReturn:
    case kTokenTypeId:
    case kTokenDir:
    case kTokenExist:
    case kTokenFn:
    case kTokenCase:
    case kTokenWhen:
    case kTokenAssert:
    case kTokenAssertR:
    case kTokenQuit:
    case kTokenEnd:
    case kTokenContinue:
    case kTokenBreak:
    case kTokenElse:
    case kTokenIf:
    case kTokenElif:
    case kTokenWhile:
      result = true;
    default:
      break;
    }

    return result;
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

  Message Module::Run(bool create_container, string name) {
    Message result;
    MachCtlBlk *blk = new MachCtlBlk();
    IR *ir = nullptr;
    
    health_ = true;

    if (create_container) management::CreateContainer();

    if (storage_.empty()) return result;

    while (blk->current < storage_.size()) {
      if (!health_) break;

      blk->last_index = (blk->current == storage_.size() - 1);
      
      if (NeedSkipping(blk->mode)) {
        result = Message();
        if (!SkippingStrategy(blk)) {
          trace::AddEvent(
            Message(kCodeIllegalParam, "End token is not found.", kStateError)
            .SetIndex(blk->current)
          );
          break;
        }
      }

      ir = &storage_[blk->current];
      result = IRProcessing(*ir, name, blk);
      
      switch (result.GetLevel()) {
      case kStateError:
        health_ = false;
        trace::AddEvent(result.SetIndex(storage_[blk->current].GetIndex()));
        continue;
        break;
      case kStateWarning:
        trace::AddEvent(result.SetIndex(storage_[blk->current].GetIndex()));
        break;
      default:break;
      }

      const auto code = result.GetCode();
      const auto detail = result.GetDetail();

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

    if (msg.GetCode() >= kCodeSuccess) {
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

