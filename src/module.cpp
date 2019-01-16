#include "module.h"

namespace kagami {
  shared_ptr<void> FakeCopy(shared_ptr<void> target) {
    return target;
  }

  shared_ptr<void> NullCopy(shared_ptr<void> target) {
    return make_shared<int>(0);
  }

  string RealString(const string &src) {
    string result = src;
    if (util::IsString(result)) result = util::GetRawString(result);
    return result;
  }

  void CopyObject(Object &dest, Object &src) {
    dest.Set(management::type::GetObjectCopy(src), src.GetTypeId(),
      src.GetMethods());
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
  //from https://www.yasuhisay.info/entry/20090722/1248245439
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

  /* Packaging entry into object and act as a object */
  inline Object GetFunctionObject(string id, string domain) {
    Object obj;
    auto ent = management::Order(id, domain);
    if (ent.Good()) {
      obj.Set(make_shared<Entry>(ent), kTypeIdFunction, kFunctionMethods);
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
      if (type != TokenTypeEnum::T_BLANK && exempt_blank_char) {
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
      util::GetTokenType(toString(data.back())) == TokenTypeEnum::T_BLANK) {
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

  map<string, Module> &GetFunctionBase() {
    static map<string, Module> base;
    return base;
  }

  /* Add new user-defined function */
  inline void AddFunction(string id, vector<IR> proc, vector<string> parms) {
    auto &base = GetFunctionBase();
    base[id] = Module(proc).SetParameters(parms).SetFunc();
    management::AddEntry(Entry(FunctionTunnel, id, parms));
  }

  inline Module *GetFunction(string id) {
    Module *module = nullptr;
    auto &base = GetFunctionBase();
    auto it = base.find(id);
    if (it != base.end()) module = &(it->second);
    return module;
  }

  /* Accept arguments from entry and deliver to function */
  Message FunctionTunnel(ObjectMap &p) {
    Message msg;
    Object &func_id = p[kStrUserFunc];
    string id = *static_pointer_cast<string>(func_id.Get());
    Module *mach = GetFunction(id);
    if (mach != nullptr) {
      msg = mach->RunAsFunction(p);
    }
    return msg;
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

  void MachCtlBlk::ConditionRoot(bool value) {
    mode_stack.push(mode);
    management::CreateContainer();
    mode = value ? kModeCondition : kModeNextCondition;
    condition_stack.push(value);
  }

  void MachCtlBlk::ConditionBranch(bool value) {
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

  bool MachCtlBlk::ConditionLeaf() {
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
        }
      }
    }
    else {
      result = false;
    }

    return result;
  }

  void MachCtlBlk::LoopHead(bool value) {
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
      if (cycle_tail.empty()) {
        current = cycle_tail.top();
      }
    }
  }
  
  void MachCtlBlk::End() {
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
  }

  void MachCtlBlk::Continue() {
    while (!mode_stack.empty() && mode != kModeCycle) {
      if (mode == kModeCondition || mode == kModeCase) {
        condition_stack.pop();
        nest_head_count += 1;
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
        nest_head_count += 1;
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
    nest_head_count = 0;
    error_string.clear();
    while (!cycle_nest.empty()) cycle_nest.pop();
    while (!cycle_tail.empty()) cycle_tail.pop();
    while (!mode_stack.empty()) mode_stack.pop();
    while (!condition_stack.empty()) condition_stack.pop();
    def_head.clear();
    def_head.shrink_to_fit();
    recursion_map.clear();
  }

  Object IRWorker::MakeObject(Argument &arg, bool checking) {
    Object obj, obj_domain;
    ObjectPointer ptr;
    bool state = true;

    switch (arg.domain.type) {
    case AT_RET:
      if (!returning_base.empty()) {
        obj_domain = returning_base.front();
      }
      else {
        error_returning = true;
        error_string = "Returning base error.";
        state = false;
      }
      break;
    case AT_OBJECT:
      ptr = management::FindObject(arg.domain.data);
      if (ptr != nullptr) {
        obj_domain.Ref(*ptr);
      }
      else {
        obj_domain = GetFunctionObject(arg.domain.data, kTypeIdNull);
        if (obj_domain.Get() == nullptr) {
          error_obj_checking = true;
          error_string = "Object is not found - " + arg.data;
          state = false;
        }
      }
      break;
    default:
      break;
    }

    if (!state) return Object();

    switch (arg.type) {
    case AT_NORMAL:
      obj.Set(make_shared<string>(arg.data), kTypeIdRawString, kRawStringMethods);
      break;
    case AT_OBJECT:
      ptr = management::FindObject(arg.data);
      if (ptr != nullptr) {
        obj.Ref(*ptr);
      }
      else {
        obj = GetFunctionObject(arg.data, obj_domain.GetTypeId());
        if (obj.GetTypeId() == kTypeIdNull) {
          error_obj_checking = true;
          error_string = "Object is not found - " + arg.data;
        }
      }
      break;
    case AT_RET:
      if (!returning_base.empty()) {
        obj = returning_base.front();
        if ((!is_assert && !checking) || is_assert_r)
          returning_base.pop_front();
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

  void IRWorker::AssemblingForAutoSized(Entry &ent, deque<Argument> parms, ObjectMap &obj_map) {
    size_t idx = 0;
    size_t va_arg_size;
    size_t count = 0;
    auto ent_args = ent.GetArguments();
    auto va_arg_head = ent_args.back();
    auto is_method = (ent.GetFlag() == kFlagMethod);

    while (idx < ent_args.size() - 1) {
      obj_map.Input(ent_args[idx],
        MakeObject(parms[idx]));
      idx += 1;
    }

    is_method ?
      va_arg_size = parms.size() - 1 :
      va_arg_size = parms.size();

    while (idx < va_arg_size) {
      obj_map.Input(va_arg_head + to_string(count),
        MakeObject(parms[idx]));
      count += 1;
      idx += 1;
    }

    obj_map.Input(kStrVaSize, Object(to_string(count), T_INTEGER));
  }

  void IRWorker::AssemblingForAutoFilling(Entry &ent, deque<Argument> parms, ObjectMap &obj_map) {
      size_t idx = 0;
      auto ent_args = ent.GetArguments();
      auto is_method = (ent.GetFlag() == kFlagMethod);

      while (idx < ent_args.size()) {
        if (idx >= parms.size()) break;
        if (idx >= parms.size() - 1 && is_method) break;
        obj_map.Input(ent_args[idx], MakeObject(parms[idx]));
        idx += 1;
      }
  }

  void IRWorker::AssemblingForNormal(Entry &ent, deque<Argument> parms, ObjectMap &obj_map) {
    size_t idx = 0;
    auto ent_args = ent.GetArguments();
    auto is_method = (ent.GetFlag() == kFlagMethod);
    bool state = true;

    while (idx < ent_args.size()) {
      if (idx >= parms.size()) {
        state = false;
        break;
      }
      obj_map.Input(ent_args[idx], MakeObject(parms[idx]));
      idx += 1;
    }

    if (!state) {
      error_assembling = true;
      error_string = "Required argument count is " +
        to_string(ent_args.size()) +
        ", but provided argument count is " +
        to_string(parms.size()) + ".";
    }
  }

  void IRWorker::Reset() {
    error_string.clear();
    error_returning = false;
    error_obj_checking = false;
    tail_recursion = false;
    error_assembling = false;
    is_assert = false;
    is_assert_r = false;
    deliver = false;
    msg = Message();
    returning_base.clear();
  }

  IRMaker::IRMaker(const char *path) : health(true) {
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
        trace::Log(msg);
        health = false;
        continue;
        break;
      case kStateWarning:
        trace::Log(msg);
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
  }

  void Module::ResetContainer(string func_id) {
    Object *func_sign = management::GetCurrentContainer().Find(kStrUserFunc);

    while (func_sign == nullptr) {
      management::DisposeManager();
      func_sign = management::GetCurrentContainer().Find(kStrUserFunc);
      if (func_sign != nullptr) {
        string value = GetObjectStuff<string>(*func_sign);
        if (value == func_id) break;
      }
    }
  }

  void Module::MakeFunction(size_t start, size_t end, vector<string> &def_head) {
    if (start > end) return;
    string id = def_head[0];
    vector<string> parms;
    vector<IR> proc;

    for (size_t i = 1; i < def_head.size(); ++i) {
      parms.push_back(def_head[i]);
    }
    for (size_t j = start; j <= end; ++j) {
      proc.push_back(storage_[j]);
    }
    AddFunction(id, proc, parms);
  }

  bool Module::IsBlankStr(string target) {
    if (target.empty() || target.size() == 0) return true;
    for (const auto unit : target) {
      if (unit != '\n' && unit != ' ' && unit != '\t' && unit != '\r') {
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
    bool state = true;
    IRWorker *worker = new IRWorker();

    for (size_t idx = 0; idx < action_base.size(); idx += 1) {
      obj_map.clear();

      auto &request = action_base.at(idx).first;
      auto &args = action_base.at(idx).second;

      if (request.type == RT_MACHINE && CheckGenericRequests(request.head_gen)) {
        (request.head_reg == name && idx == action_base.size() - 2
          && action_base.back().first.head_gen == GT_RETURN) ?
          worker->tail_recursion = true :
          worker->tail_recursion = false;

        state = GenericRequests(worker, request, args);

        if (worker->deliver) {
          msg = worker->msg;
          worker->deliver = false;
          worker->msg = Message();
        }
      }
      else if (request.type == RT_REGULAR || (request.type == RT_MACHINE && !CheckGenericRequests(request.head_gen))) {
        StateCode code;
        Entry ent;
        string type_id, value, detail;
        is_operator_token = util::IsOperatorToken(request.head_gen);

        if (!preprocessing && worker->tail_recursion) {
          (request.head_reg == name && idx == action_base.size() - 1
            && blk->last_index && name != "" && name != "__null__") ?
            worker->tail_recursion = true :
            worker->tail_recursion = false;
        }

        switch (request.type) {
        case RT_MACHINE:
          ent = management::GetGenericProvider(request.head_gen);
          break;
        case RT_REGULAR:
          if (request.domain.type != AT_HOLDER) {
            Object domain_obj = worker->MakeObject(request.domain, true);
            type_id = domain_obj.GetTypeId();
            ent = management::Order(request.head_reg, type_id);
            obj_map.Input(kStrObject, domain_obj);
          }
          else {
            ent = management::Order(request.head_reg);
          }
          break;
        default:
          break;
        }

        if (!ent.Good()) {
          msg = Message(kCodeIllegalCall, 
            "Function is not found - " + request.head_reg,
            kStateError);
          break;
        }

        switch (ent.GetArgumentMode()) {
        case kCodeAutoSize:
          worker->AssemblingForAutoSized(ent, args, obj_map);
          break;
        case kCodeAutoFill:
          worker->AssemblingForAutoFilling(ent, args, obj_map);
          break;
        default:
          worker->AssemblingForNormal(ent, args, obj_map);
          break;
        }

        if (worker->error_returning ||
          worker->error_obj_checking ||
          worker->error_assembling) {

          break;
        }

        if (worker->tail_recursion) {
          blk->recursion_map = obj_map;
          break;
        }

        msg = ent.Start(obj_map);
        
        if (msg.GetLevel() == kStateError) break;

        code = msg.GetCode();
        detail = msg.GetDetail();

        if (code == kCodeObject && request.head_gen != GT_TYPE_ASSERT) {
          auto object = msg.GetObj();

          if (is_operator_token && idx + 1 < action_base.size()) {
            util::IsOperatorToken(action_base[idx + 1].first.head_gen) ?
              worker->returning_base.emplace_front(object) :
              worker->returning_base.emplace_back(object);
          }
          else {
            worker->returning_base.emplace_back(object);
          }
        }
        else if (request.head_gen != GT_TYPE_ASSERT) {
          worker->returning_base.emplace_back(Object());
        }
      }
    }

    if (worker->error_returning ||
      worker->error_obj_checking ||
      worker->error_assembling) {
      msg = Message(kCodeIllegalSymbol, worker->error_string, kStateError);
    }

    if (!preprocessing && worker->tail_recursion)
      blk->tail_recursion = worker->tail_recursion;

    obj_map.clear();
    worker->returning_base.clear();
    worker->returning_base.shrink_to_fit();

    delete worker;

    return msg;
  }

  Message Module::PreProcessing() {
    IR *ir = nullptr;
    GenericTokenEnum token;
    Message result;
    bool flag = false;
    map<size_t, size_t> skipped_idx;
    using IndexPair = pair<size_t, size_t>;
    size_t nest_head_count = 0;
    size_t def_start;
    vector<string> def_head;

    for (size_t idx = 0; idx < storage_.size(); ++idx) {
      if (!health_) break;
      ir = &storage_[idx];
      token = util::GetGenericToken(ir->GetMainToken().first);
      if (token == GT_WHILE || token == GT_IF || token == GT_CASE) {
        nest_head_count++;
      }
      else if (token == GT_DEF) {
        if (flag == true) {
          result = Message(kCodeBadExpression,
            "Define function in function is not supported.",
            kStateError).SetIndex(idx);
          break;
        }
        result = IRProcessing(*ir, "", nullptr);
        def_head = util::BuildStringVector(result.GetDetail());
        def_start = idx + 1;
        flag = true;
      }
      else if (token == GT_END) {
        if (nest_head_count > 0) {
          nest_head_count--;
        }
        else {
          skipped_idx.insert(IndexPair(def_start - 1, idx));
          MakeFunction(def_start, idx - 1, def_head);
          def_head.clear();
          def_head.shrink_to_fit();
          def_start = 0;
          flag = false;
        }
      }
    }

    vector<IR> *other_sets = new vector<IR>();
    bool filter = false;
    for (size_t idx = 0; idx < storage_.size(); ++idx) {
      auto it = skipped_idx.find(idx);
      if (it != skipped_idx.end()) {
        idx = it->second;
        continue;
      }
      other_sets->emplace_back(storage_[idx]);
    }

    storage_.swap(*other_sets);
    delete other_sets;

    return result;
  }

  void Module::InitGlobalObject(bool create_container, string name) {
    if (create_container) management::CreateContainer();

    auto create = [&](string id, string value)->void {
      management::CreateObject(id, Object()
        .Set(make_shared<string>("'" + value + "'"), kTypeIdRawString, kRawStringMethods));
    };

    if (is_main_) {
      create("__name__", "__main__");
    }
    else {
      if (name != "") {
        create("__name__", name);
      }
      else {
        create("__name__", "__null__");
      }
    }

    create("__platform__", kPlatformType);
    create("__version__", kEngineVersion);
    create("__backend__", kBackendVerison);
  }

  bool Module::PredefinedMessage(Message &result, size_t mode, Token token) {
    bool judged = false;
    GenericTokenEnum gen_token = util::GetGenericToken(token.first);

    switch (mode) {
    case kModeNextCondition:
      if (management::HasTailTokenRequest(gen_token)) {
        result = Message(kStrTrue);
        judged = true;
      }
      else if (gen_token != GT_ELSE && gen_token != GT_END && gen_token != GT_ELIF) {
        result = Message(kStrPlaceHolder);
        judged = true;
      }
      break;
    case kModeCycleJump:
      if (gen_token != GT_END && gen_token != GT_IF && gen_token != GT_WHILE) {
        result = Message(kStrPlaceHolder);
        judged = true;
      }
      break;
    case kModeCaseJump:
      if (management::HasTailTokenRequest(gen_token)) {
        result = Message(kStrTrue);
        judged = true;
      }
      else if (gen_token != GT_WHEN && gen_token != GT_END && gen_token != GT_ELSE) {
        result = Message(kStrPlaceHolder);
        judged = true;
      }
      break;
    default:
      break;
    }

    return judged;
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

  bool Module::BindAndSet(IRWorker *worker, deque<Argument> args) {
    bool result = true;
    auto dest = worker->MakeObject(args[0]);
    auto src = worker->MakeObject(args[1]);

    if (dest.IsRef()) {
      CopyObject(dest, src);
    }
    else {
      string id = GetObjectStuff<string>(dest);

      if (util::GetTokenType(id) == T_GENERIC) {
        ObjectPointer real_dest = management::FindObject(id);

        if (real_dest != nullptr) {
          CopyObject(*real_dest, src);
        }
        else {
          Object obj(management::type::GetObjectCopy(src), src.GetTypeId(), src.GetMethods());
          ObjectPointer result = management::CreateObject(id, obj);
          if (result == nullptr) {
            worker->error_string = "Object cration is failed.";
            result = false;
          }
        }
      }
      else {
        worker->error_string = "Invalid bind operation.";
        result = false;
      }
    }

    return result;
  }

  void Module::Nop(IRWorker *worker, deque<Argument> args) {
    if (!args.empty()) {
      auto obj = worker->MakeObject(args.back());
      worker->returning_base.emplace_back(obj);
    }
  }

  void Module::ArrayMaker(IRWorker *worker, deque<Argument> args) {
    shared_ptr<vector<Object>> base = make_shared<vector<Object>>();
    
    if (!args.empty()) {
      for (size_t idx = 0; idx < args.size(); idx += 1) {
        base->emplace_back(worker->MakeObject(args[idx]));
      }
    }

    Object obj(base, kTypeIdArrayBase, management::type::GetMethods(kTypeIdArrayBase));
    obj.SetConstructorFlag();
    worker->returning_base.emplace_back(obj);
  }

  void Module::ReturnOperator(IRWorker *worker, deque<Argument> args) {
    auto &container = management::GetCurrentContainer();
    if (args.size() > 1) {
      shared_ptr<vector<Object>> base = make_shared<vector<Object>>();
      for (size_t idx = 0; idx < args.size(); idx += 1) {
        base->emplace_back(worker->MakeObject(args[idx]));
      }
      Object obj(base, kTypeIdArrayBase, management::type::GetMethods(kTypeIdArrayBase));
      container.Add(kStrRetValue,
        Object(obj.Get(), obj.GetTypeId(), obj.GetMethods()));
    }
    else if (args.size() == 1) {
      auto obj = worker->MakeObject(args[0]);
      container.Add(kStrRetValue,
        Object(obj.Get(), obj.GetTypeId(), obj.GetMethods()));
    }
    worker->msg = Message(kCodeReturn, "");
    worker->deliver = true;
  }

  bool Module::GetTypeId(IRWorker *worker, deque<Argument> args) {
    bool result = true;
    
    if (args.size() > 1) {
      shared_ptr<vector<Object>> base = make_shared<vector<Object>>();
      for (size_t idx = 0; idx < args.size(); idx += 1) {
        auto obj = worker->MakeObject(args[idx]);
        Object value_obj(obj.GetTypeId(), util::GetTokenType(obj.GetTypeId()));
        base->emplace_back(value_obj);
      }
      Object ret_obj(base, kTypeIdArrayBase, management::type::GetMethods(kTypeIdArrayBase));
      ret_obj.SetConstructorFlag();
      worker->returning_base.emplace_back(ret_obj);
    }
    else if (args.size() == 1){
      auto obj = worker->MakeObject(args[0]);
      Object value_obj(obj.GetTypeId(), util::GetTokenType(obj.GetTypeId()));
      worker->returning_base.push_back(value_obj);
    }
    else {
      worker->error_string = "Empty argument list.";
      result = false;
    }

    return result;
  }

  bool Module::GetMethods(IRWorker *worker, deque<Argument> args) {
    bool result = true;

    if (!args.empty()) {
      Object obj = worker->MakeObject(args[0]);
      auto vec = util::BuildStringVector(obj.GetMethods());
      shared_ptr<vector<Object>> base = make_shared<vector<Object>>();

      for (const auto &unit : vec) {
        base->emplace_back(Object(make_shared<string>(unit), kTypeIdString,
          management::type::GetMethods(kTypeIdString)));
      }

      Object ret_obj(base, kTypeIdArrayBase, kArrayBaseMethods);
      ret_obj.SetConstructorFlag();
      worker->returning_base.emplace_back(ret_obj);
    }
    else {
      worker->error_string = "Empty argument list.";
      result = false;
    }

    return result;
  }

  bool Module::Exist(IRWorker *worker, deque<Argument> args) {
    bool result = true;

    if (args.size() == 2) {
      Object obj = worker->MakeObject(args[0]);
      Object str_obj = worker->MakeObject(args[1]);
      Object ret_obj;
      string target_str = RealString(GetObjectStuff<string>(str_obj));
      util::FindInStringGroup(target_str, obj.GetMethods()) ?
        ret_obj = Object(kStrTrue, T_BOOLEAN) :
        ret_obj = Object(kStrFalse, T_BOOLEAN);

      worker->returning_base.emplace_back(ret_obj);
    }
    else if (args.size() > 2) {
      worker->error_string = "Too many arguments.";
      result = false;
    }
    else if (args.size() < 2) {
      worker->error_string = "Too few arguments.";
      result = false;
    }

    return result;
  }

  bool Module::Define(IRWorker *worker, deque<Argument> args) {
    bool result = true;

    if (!args.empty()) {
      vector<string> def_head;

      for (size_t idx = 0; idx < args.size(); idx += 1) {
        def_head.emplace_back(args[idx].data);
      }

      string def_head_string = util::CombineStringVector(def_head);
      worker->deliver = true;
      worker->msg = Message(kCodeDefineSign, def_head_string);
    }
    else {
      worker->error_string = "Empty argument list.";
      result = false;
    }

    return result;
  }

  bool Module::Case(IRWorker *worker, deque<Argument> args) {
    bool result = true;

    if (!args.empty()) {
      Object obj = worker->MakeObject(args[0]);

      if (!IsStringObject(obj)) {
        result = false;
        worker->error_string = "Case-when is not supported for this object.";
      }
      else {
        auto copy = management::type::GetObjectCopy(obj);
        Object base(copy, obj.GetTypeId(), obj.GetMethods());
        management::CreateObject("__case", base);
        worker->deliver = true;
        worker->msg = Message().SetCode(kCodeCase);
      }
    }
    else {
      result = false;
      worker->error_string = "Empty argument list.";
    }

    return result;
  }

  bool Module::When(IRWorker *worker, deque<Argument> args) {
    bool result = true;

    if (!args.empty()) {
      ObjectPointer case_head = management::FindObject("__case");
      string case_content = GetObjectStuff<string>(*case_head);
      bool state = true, found = false;

      for (size_t idx = 0; idx < args.size(); idx += 1) {
        auto obj = worker->MakeObject(args[idx]);
        if (!IsStringObject(obj)) {
          state = false;
          break;
        }

        if (case_content == GetObjectStuff<string>(obj)) {
          found = true;
          break;
        }
      }

      if (state) {
        worker->deliver = true;
        worker->msg = found ? 
          Message(kCodeWhen, kStrTrue) :
          Message(kCodeWhen, kStrFalse);
      }
      else {
        worker->error_string = "Case-when is not supported for non-string object.";
        result = false;
      }
    }
    else {
      worker->error_string = "Empty argument list.";
      result = false;
    }

    return result;
  }

  bool Module::DomainAssert(IRWorker *worker, deque<Argument> args, bool returning) {
    bool result = true;

    Object obj = worker->MakeObject(args[0]);
    Object id_obj = worker->MakeObject(args[1]);
    string id = GetObjectStuff<string>(id_obj);

    result = util::FindInStringGroup(id, obj.GetMethods());

    if (!result) {
      worker->error_string = "Method/Member is not found. - " + id;
      return result;
    } 
    
    ObjectPointer ret_ptr = management::FindObject(id);
    
    if (ret_ptr != nullptr) {
      if (returning) worker->returning_base.emplace_back(*ret_ptr);
    }
    else {
      Object ent_obj;
      auto ent = management::Order(id, obj.GetTypeId());
      if (ent.Good()) {
        if (returning) {
          ent_obj.Set(make_shared<Entry>(ent), kTypeIdFunction, kFunctionMethods);
          worker->returning_base.emplace_back(ent_obj);
        }
      }
      else {
        result = false;
        worker->error_string = "Method/Member is not found. - " + id;
      }
    }

    return result;
  }

  inline void MakeCode(StateCode code, IRWorker *worker) {
    worker->deliver = true;
    worker->msg = Message(code, "");
  }

  void Module::Quit(IRWorker *worker) {
    MakeCode(kCodeQuit, worker);
  }

  void Module::End(IRWorker *worker) {
    MakeCode(kCodeTailSign, worker);
  }

  void Module::Continue(IRWorker *worker) {
    MakeCode(kCodeContinue, worker);
  }

  void Module::Break(IRWorker *worker) {
    MakeCode(kCodeBreak, worker);
  }

  void Module::Else(IRWorker *worker) {
    MakeCode(kCodeConditionLeaf, worker);
  }

  bool Module::ConditionAndLoop(IRWorker *worker, deque<Argument> args, StateCode code) {
    bool result = true;
    if (args.size() == 1) {
      Object obj = worker->MakeObject(args[0]);
      worker->deliver = true;
      if (obj.GetTypeId() != kTypeIdRawString && obj.GetTypeId() != kTypeIdNull) {
        worker->msg = Message(code, kStrTrue);
      }
      else {
        string state_str = GetObjectStuff<string>(obj);
        if (state_str == kStrTrue || state_str == kStrFalse) {
          worker->msg = Message(code, state_str);
        }
        else {
          auto type = util::GetTokenType(state_str);

          bool state = false;
          switch (type) {
          case T_INTEGER:
            state = (stoi(state_str) != 0);
            break;
          case T_FLOAT:
            state = (stod(state_str) != 0.0);
            break;
          case T_STRING:
            state = (RealString(state_str).size() > 0);
            break;
          default:
            break;
          }

          worker->msg = Message(code, state ? kStrTrue : kStrFalse);
        }
      }
    }
    else if (args.empty()) {
      worker->error_string = "Too few arguments.";
      result = false;
    }
    else {
      worker->error_string = "Too many arguments.";
      result = false;
    }

    return result;
  }

  bool Module::GenericRequests(IRWorker *worker, Request &request, deque<Argument> &args) {
    auto &token = request.head_gen;
    bool result = true;

    switch (token) {
    case GT_BIND:
      result = BindAndSet(worker, args);
      break;
    case GT_NOP:
      Nop(worker, args);
      break;
    case GT_ARRAY:
      ArrayMaker(worker, args);
      break;
    case GT_RETURN:
      ReturnOperator(worker, args);
      break;
    case GT_TYPEID:
      GetTypeId(worker, args);
      break;
    case GT_DIR:
      result = GetMethods(worker, args);
      break;
    case GT_EXIST:
      result = Exist(worker, args);
      break;
    case GT_DEF:
      result = Define(worker, args);
      break;
    case GT_CASE:
      result = Case(worker, args);
      break;
    case GT_WHEN:
      result = When(worker, args);
      break;
    case GT_TYPE_ASSERT:
      result = DomainAssert(worker, args, false);
      break;
    case GT_ASSERT_R:
      result = DomainAssert(worker, args, true);
    case GT_QUIT:
      Quit(worker);
      break;
    case GT_END:
      End(worker);
      break;
    case GT_CONTINUE:
      Continue(worker);
      break;
    case GT_BREAK:
      Break(worker);
      break;
    case GT_ELSE:
      Else(worker);
    case GT_IF:
      result = ConditionAndLoop(worker, args, kCodeConditionRoot);
      break;
    case GT_ELIF:
      result = ConditionAndLoop(worker, args, kCodeConditionBranch);
      break;
    case GT_WHILE:
      result = ConditionAndLoop(worker, args, kCodeHeadSign);
      break;
    default:
      break;
    }

    return result;
  }

  bool Module::CheckGenericRequests(GenericTokenEnum token) {
    bool result = false;

    switch (token) {
    case GT_BIND:
    case GT_NOP:
    case GT_ARRAY:
    case GT_RETURN:
    case GT_TYPEID:
    case GT_DIR:
    case GT_EXIST:
    case GT_DEF:
    case GT_CASE:
    case GT_WHEN:
    case GT_TYPE_ASSERT:
    case GT_ASSERT_R:
    case GT_QUIT:
    case GT_END:
    case GT_CONTINUE:
    case GT_BREAK:
    case GT_ELSE:
    case GT_IF:
    case GT_ELIF:
    case GT_WHILE:
      result = true;
    default:
      break;
    }

    return result;
  }

  Message Module::Run(bool create_container, string name) {
    Message result;
    MachCtlBlk *blk = new MachCtlBlk();
    IR *ir = nullptr;
    bool judged = false;
    
    health_ = true;

    if (storage_.empty()) return result;

    InitGlobalObject(create_container, name);
    if (result.GetCode() < kCodeSuccess) return result;

    while (blk->current < storage_.size()) {
      if (!health_) break;

      blk->last_index = (blk->current == storage_.size() - 1);
      ir = &storage_[blk->current];

      judged = PredefinedMessage(result, blk->mode, ir->GetMainToken());
      
      if (!judged) result = IRProcessing(*ir, name, blk);

      switch (result.GetLevel()) {
      case kStateError:
        health_ = false;
        trace::Log(result.SetIndex(storage_[blk->current].GetIndex()));
        continue;
        break;
      case kStateWarning:
        trace::Log(result.SetIndex(storage_[blk->current].GetIndex()));
        break;
      default:break;
      }

      const auto code  = result.GetCode();
      const auto detail = result.GetDetail();

      if (blk->tail_recursion) {
        TailRecursionActions(blk, name);
        continue;
      }

      if (code == kCodeReturn) {
        break;
      }

      switch (code) {
      case kCodeContinue:
        blk->Continue();
        break;
      case kCodeBreak:
        blk->Break();
        break;
      case kCodeConditionRoot:
        blk->ConditionRoot(GetBooleanValue(detail));
        break;
      case kCodeConditionBranch:
        if (blk->nest_head_count > 0) break;
        blk->ConditionBranch(GetBooleanValue(detail));
        break;
      case kCodeConditionLeaf:
        if (blk->nest_head_count > 0) break;
        health_ = blk->ConditionLeaf();
        break;
      case kCodeCase:
        blk->Case();
        break;
      case kCodeWhen:
        blk->When(GetBooleanValue(detail));
        break;
      case kCodeHeadSign:
        blk->LoopHead(GetBooleanValue(detail));
        break;
      case kCodeHeadPlaceholder:
        blk->nest_head_count++;
        break;
      case kCodeTailSign:
        if (blk->nest_head_count > 0) {
          blk->nest_head_count--;
          break;
        }
        blk->End();
        break;
      default:break;
      }

      if (blk->runtime_error) break;

      ++blk->current;
      if (judged) judged = false;
    }

    if (blk->runtime_error) {
      trace::Log(Message(kCodeBadExpression, blk->error_string, kStateError)
        .SetIndex(blk->current));
    }

    if (create_container) management::DisposeManager();

    delete blk;
    return result;
  }

  Message Module::RunAsFunction(ObjectMap &p) {
    Message msg;
    auto &base = management::CreateContainer();
    string func_id = p.Get<string>(kStrUserFunc);

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
        obj.Copy(*ret);
        msg.SetObject(obj);
      }
    }

    Object *func_sign = management::GetCurrentContainer().Find(kStrUserFunc);

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

