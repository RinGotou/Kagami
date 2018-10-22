#include "machine.h"

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

  inline Object GetFunctionObject(string id, string domain) {
    Object obj;
    auto ent = entry::Order(id, domain);
    if (ent.Good()) {
      obj.Set(make_shared<Entry>(ent), kTypeIdFunction, kFunctionMethods, false);
    }
    return obj;
  }

  string IndentationAndCommentProc(string target) {
    if (target == "") return "";
    string data;
    char current, last;
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

  map<string, Machine> &GetFunctionBase() {
    static map<string, Machine> base;
    return base;
  }

  inline void AddFunction(string id, vector<Meta> proc, vector<string> parms) {
    auto &base = GetFunctionBase();
    base[id] = Machine(proc).SetParameters(parms).SetFunc();
    entry::AddEntry(Entry(FunctionTunnel, id, parms));
  }

  inline Machine *GetFunction(string id) {
    Machine *machine = nullptr;
    auto &base = GetFunctionBase();
    auto it = base.find(id);
    if (it != base.end()) machine = &(it->second);
    return machine;
  }

  Message FunctionTunnel(ObjectMap &p) {
    Message msg;
    Object &func_id = p[kStrUserFunc];
    string id = *static_pointer_cast<string>(func_id.Get());
    Machine *mach = GetFunction(id);
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

  Message Calling(Activity activity, string parmStr, vector<Object> objects) {
    vector<string> parms = util::BuildStringVector(parmStr);
    ObjectMap obj_map;
    for (size_t i = 0; i < parms.size(); i++) {
      obj_map.insert(NamedObject(parms[i], objects[i]));
    }
    return activity(obj_map);
  }

  void Machine::ResetBlock(MachCtlBlk *blk) {
    blk->mode = kModeNormal;
    blk->nest_head_count = 0;
    blk->current = 0;
    blk->mode = kModeNormal;
    blk->def_start = 0;
    blk->s_continue = false;
    blk->s_break = false;
    blk->last_index = false;
    blk->tail_recursion = false;
    blk->tail_call = false;
    blk->runtime_error = false;
  }

  void Machine::ResetContainer(string func_id) {
    Object *func_sign = entry::GetCurrentContainer().Find(kStrUserFunc);

    while (func_sign == nullptr) {
      entry::DisposeManager();
      func_sign = entry::GetCurrentContainer().Find(kStrUserFunc);
      if (func_sign != nullptr) {
        string value = GetObjectStuff<string>(*func_sign);
        if (value == func_id) break;
      }
    }
  }

  void Machine::CaseHead(Message &msg, MachCtlBlk *blk) {
    entry::CreateContainer();
    blk->mode_stack.push(blk->mode);
    blk->mode = kModeCaseJump;
    blk->condition_stack.push(false);
  }

  void Machine::WhenHead(bool value, MachCtlBlk *blk) {
    if (!blk->condition_stack.empty()) {
      if (blk->mode == kModeCase && blk->condition_stack.top() == true) {
        blk->mode = kModeCaseJump;
      }
      else if (value == true && blk->condition_stack.top() == false) {
        blk->mode = kModeCase;
        blk->condition_stack.top() = true;
      }
    }
  }

  void Machine::ConditionRoot(bool value, MachCtlBlk *blk) {
    blk->mode_stack.push(blk->mode);
    entry::CreateContainer();
    if (value == true) {
      blk->mode = kModeCondition;
      blk->condition_stack.push(true);
    }
    else {
      blk->mode = kModeNextCondition;
      blk->condition_stack.push(false);
    }
  }

  void Machine::ConditionBranch(bool value, MachCtlBlk *blk) {
    if (!blk->condition_stack.empty()) {
      if (blk->condition_stack.top() == false && blk->mode == kModeNextCondition
        && value == true) {
        entry::CreateContainer();
        blk->mode = kModeCondition;
        blk->condition_stack.top() = true;
      }
      else if (blk->condition_stack.top() == true && blk->mode == kModeCondition) {
        blk->mode = kModeNextCondition;
      }
    }
    else {
      //msg = Message
      health_ = false;
    }
  }

  void Machine::ConditionLeaf(MachCtlBlk *blk) {
    if (!blk->condition_stack.empty()) {
      if (blk->condition_stack.top() == true) {
        switch (blk->mode) {
        case kModeCondition:
        case kModeNextCondition:
          blk->mode = kModeNextCondition;
          break;
        case kModeCase:
        case kModeCaseJump:
          blk->mode = kModeCaseJump;
          break;
        }
      }
      else {
        entry::CreateContainer();
        blk->condition_stack.top() = true;
        switch (blk->mode) {
        case kModeNextCondition:
          blk->mode = kModeCondition;
          break;
        case kModeCaseJump:
          blk->mode = kModeCase;
          break;
        }
      }
    }
    else {
      //msg = Message
      health_ = false;
    }
  }

  void Machine::HeadSign(bool value, MachCtlBlk *blk) {
    if (blk->cycle_nest.empty()) {
      blk->mode_stack.push(blk->mode);
      entry::CreateContainer();
    }
    else {
      if (blk->cycle_nest.top() != blk->current - 1) {
        blk->mode_stack.push(blk->mode);
        entry::CreateContainer();
      }
    }
    if (value == true) {
      blk->mode = kModeCycle;
      if (blk->cycle_nest.empty()) {
        blk->cycle_nest.push(blk->current - 1);
      }
      else if (blk->cycle_nest.top() != blk->current - 1) {
        blk->cycle_nest.push(blk->current - 1);
      }
    }
    else if (value == false) {
      blk->mode = kModeCycleJump;
      if (!blk->cycle_tail.empty()) {
        blk->current = blk->cycle_tail.top();
      }
    }
    else {
      health_ = false;
    }
  }

  void Machine::TailSign(MachCtlBlk *blk) {
    if (blk->mode == kModeCondition || blk->mode == kModeNextCondition) {
      blk->condition_stack.pop();
      blk->mode = blk->mode_stack.top();
      blk->mode_stack.pop();
      entry::DisposeManager();
    }
    else if (blk->mode == kModeCycle || blk->mode == kModeCycleJump) {
      switch (blk->mode) {
      case kModeCycle:
        if (blk->cycle_tail.empty() || blk->cycle_tail.top() != blk->current - 1) {
          blk->cycle_tail.push(blk->current - 1);
        }
        blk->current = blk->cycle_nest.top();
        entry::GetCurrentContainer().clear();
        break;
      case kModeCycleJump:
        if (blk->s_continue) {
          if (blk->cycle_tail.empty() || blk->cycle_tail.top() != blk->current - 1) {
            blk->cycle_tail.push(blk->current - 1);
          }
          blk->current = blk->cycle_nest.top();
          blk->mode = kModeCycle;
          blk->mode_stack.top() = blk->mode;
          blk->s_continue = false;
          entry::GetCurrentContainer().clear();
        }
        else {
          if (blk->s_break) blk->s_break = false;
          blk->mode = blk->mode_stack.top();
          blk->mode_stack.pop();
          if (!blk->cycle_nest.empty()) blk->cycle_nest.pop();
          if (!blk->cycle_tail.empty()) blk->cycle_tail.pop();
          entry::DisposeManager();
        }
        break;
      default:break;
      }
    }
    else if (blk->mode == kModeCase || blk->mode == kModeCaseJump) {
      blk->condition_stack.pop();
      blk->mode = blk->mode_stack.top();
      blk->mode_stack.pop();
      entry::DisposeManager();
    }
  }

  void Machine::Continue(MachCtlBlk *blk) {
    while (!blk->mode_stack.empty() && blk->mode != kModeCycle) {
      if (blk->mode == kModeCondition || blk->mode == kModeCase) {
        blk->condition_stack.pop();
        blk->nest_head_count++;
      }
      blk->mode = blk->mode_stack.top();
      blk->mode_stack.pop();
    }
    if (blk->mode == kModeCase) {
      blk->mode = kModeCycleJump;
      blk->s_continue = true;
    }
    else {
      blk->runtime_error = true;
      blk->error_string = "Illegal 'continue' operation.";
    }
  }

  void Machine::Break(MachCtlBlk *blk) {
    while (!blk->mode_stack.empty() && blk->mode != kModeCycle) {
      if (blk->mode == kModeCondition || blk->mode == kModeCase) {
        blk->condition_stack.pop();
        blk->nest_head_count++;
      }
      blk->mode = blk->mode_stack.top();
      blk->mode_stack.pop();
    }
    if (blk->mode == kModeCycle) {
      blk->mode = kModeCycleJump;
      blk->s_break = true;
    }
    else {
      blk->runtime_error = true;
      blk->error_string = "Illegal 'break' operation.";
    }
  }

  void Machine::MakeFunction(size_t start, size_t end, vector<string> &def_head) {
    if (start > end) return;
    string id = def_head[0];
    vector<string> parms;
    vector<Meta> proc;

    for (size_t i = 1; i < def_head.size(); ++i) {
      parms.push_back(def_head[i]);
    }
    for (size_t j = start; j <= end; ++j) {
      proc.push_back(storage_[j]);
    }
    AddFunction(id, proc, parms);
  }

  bool Machine::IsBlankStr(string target) {
    if (target.empty() || target.size() == 0) return true;
    for (const auto unit : target) {
      if (unit != '\n' && unit != ' ' && unit != '\t' && unit != '\r') {
        return false;
      }
    }
    return true;
  }

  Object Machine::MakeObject(Argument &arg, MetaWorkBlock *meta_blk, bool checking) {
    Object obj, obj_domain;
    ObjectPointer ptr;
    bool state = true;

    switch (arg.domain.type) {
    case AT_RET:
      if (!meta_blk->returning_base.empty()) {
        obj_domain = meta_blk->returning_base.front();
      }
      else {
        meta_blk->error_returning = true;
        meta_blk->error_string = "Returning base error.";
        state = false;
      }
      break;
    case AT_OBJECT:
      ptr = entry::FindObject(arg.domain.data);
      if (ptr != nullptr) {
        obj_domain.Ref(*ptr);
      }
      else {
        obj_domain = GetFunctionObject(arg.domain.data, kTypeIdNull);
        if (obj_domain.Get() == nullptr) {
          meta_blk->error_obj_checking = true;
          meta_blk->error_string = "Object is not found - " + arg.data;
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
      obj.Manage(arg.data, arg.tokenType);
      break;
    case AT_OBJECT:
      ptr = entry::FindObject(arg.data,obj_domain.GetTypeId());
      if (ptr != nullptr) {
        obj.Ref(*ptr);
      }
      else {
        obj = GetFunctionObject(arg.data, obj_domain.GetTypeId());
        if (obj.GetTypeId() == kTypeIdNull) {
          meta_blk->error_obj_checking = true;
          meta_blk->error_string = "Object is not found - " + arg.data;
        }
      }
      break;
    case AT_RET:
      if (!meta_blk->returning_base.empty()) {
        obj = meta_blk->returning_base.front();
        if ((!meta_blk->is_assert && !checking) || meta_blk->is_assert_r) 
          meta_blk->returning_base.pop_front();
      }
      else {
        meta_blk->error_returning = true;
        meta_blk->error_string = "Returning base error.";
      }
      break;
    default:
      break;
    }

    return obj;
  }

  void Machine::ResetMetaWorkBlock(MetaWorkBlock *meta_blk) {
    meta_blk->error_returning = false;
    meta_blk->error_obj_checking = false;
    meta_blk->tail_recursion = false;
    meta_blk->error_assembling = false;
    meta_blk->is_assert = false;
    meta_blk->is_assert_r = false;
  }

  void Machine::AssemblingForAutosized(Instruction &inst, 
    ObjectMap &obj_map, 
    MetaWorkBlock *meta_blk) {

    size_t idx = 0;
    size_t va_arg_size;
    size_t count = 0;
    auto &ent = inst.first;
    auto &parms = inst.second;
    auto ent_args = ent.GetArguments();
    auto va_arg_head = ent_args.back();
    auto is_method = (ent.GetFlag() == kFlagMethod);

    while (idx < ent_args.size() - 1) {
      obj_map.Input(ent_args[idx], 
        MakeObject(parms[idx], meta_blk));
      idx += 1;
    }

    is_method ?
      va_arg_size = parms.size() - 1 :
      va_arg_size = parms.size();

    while (idx < va_arg_size) {
      obj_map.Input(va_arg_head + to_string(count), 
        MakeObject(parms[idx], meta_blk));
      count += 1;
      idx += 1;
    }

    obj_map.Input(kStrSize, Object().Manage(to_string(count), T_INTEGER));

    if(is_method) obj_map.Input(kStrObject, MakeObject(parms.back(), meta_blk));
  }

  void Machine::AssemblingForAutoFilling(Instruction &inst, 
    ObjectMap &obj_map, 
    MetaWorkBlock *meta_blk) {

    size_t idx = 0;
    auto &ent = inst.first;
    auto &parms = inst.second;
    auto ent_args = ent.GetArguments();
    auto is_method = (ent.GetFlag() == kFlagMethod);

    while (idx < ent_args.size()) {
      if (idx >= parms.size()) break;
      if (idx >= parms.size() - 1 && is_method) break;
      obj_map.Input(ent_args[idx], MakeObject(parms[idx], meta_blk));
      idx += 1;
    }

    if (is_method) obj_map.Input(kStrObject, MakeObject(parms.back(), meta_blk));
  }

  void Machine::AssemblingForNormal(Instruction &inst, 
    ObjectMap &obj_map, 
    MetaWorkBlock *meta_blk) {

    size_t idx = 0;
    auto &ent = inst.first;
    auto &parms = inst.second;
    auto ent_args = ent.GetArguments();
    auto is_method = (ent.GetFlag() == kFlagMethod);
    bool state = true;

    while (idx < ent_args.size()) {
      if (idx >= parms.size()) {
        state = false;
        break;
      }
      obj_map.Input(ent_args[idx], MakeObject(parms[idx], meta_blk));
      idx += 1;
    }

    if (!state) {
      meta_blk->error_assembling = true;
      meta_blk->error_string = "Required argument count is " +
        to_string(ent_args.size()) +
        ", but provided argument count is " +
        to_string(parms.size()) + ".";
    }
    else {
      if (is_method) obj_map.Input(kStrObject, MakeObject(parms.back(), meta_blk));
    }
  }

  Message Machine::MetaProcessing(Meta &meta, string name, MachCtlBlk *blk) {
    int code;
    string id, type_id, va_arg_head, value, detail;
    Message msg;
    ObjectMap obj_map;
    vector<Instruction> &action_base = meta.GetContains();
    bool preprocessing = (blk == nullptr),
      is_operator_token = false;
    MetaWorkBlock *meta_blk = new MetaWorkBlock();

    auto reset = [&]()->void {
      obj_map.clear();
      id.clear();
      va_arg_head.clear();
      type_id.clear();
    };

    ResetMetaWorkBlock(meta_blk);

    for (size_t idx = 0; idx < action_base.size(); idx += 1) {
      reset();

      auto &ent = action_base[idx].first;
      auto &parms = action_base[idx].second;
      is_operator_token = entry::IsOperatorToken(ent.GetTokenEnum());
      meta_blk->is_assert = (ent.GetTokenEnum() == GT_TYPE_ASSERT ||
        ent.GetTokenEnum() == GT_ASSERT_R);
      meta_blk->is_assert_r = (ent.GetTokenEnum() == GT_ASSERT_R);

      (ent.GetId() == name && idx == action_base.size() - 2
        && action_base.back().first.GetTokenEnum() == GT_RETURN) ?
        meta_blk->tail_recursion = true :
        meta_blk->tail_recursion = false;

      if (!preprocessing && !meta_blk->tail_recursion) {
        (ent.GetId() == name && idx == action_base.size() - 1
          && blk->last_index && name != "") ?
          meta_blk->tail_recursion = true :
          meta_blk->tail_recursion = false;
      }

      if (ent.NeedRecheck()) {
        id = ent.GetId();
        ent.IsMethod() ?
          type_id = MakeObject(parms.back(), meta_blk, true).GetTypeId() :
          type_id = kTypeIdNull;

        ent = entry::Order(id, type_id);

        if (!ent.Good()) {
          msg = Message(kStrFatalError, kCodeIllegalCall, "Function not found - " + id);
          break;
        }
      }

      switch (ent.GetArgumentMode()) {
      case kCodeAutoSize:
        AssemblingForAutosized(action_base[idx], obj_map, meta_blk);
        break;
      case kCodeAutoFill:
        AssemblingForAutoFilling(action_base[idx], obj_map, meta_blk);
        break;
      default:
        AssemblingForNormal(action_base[idx], obj_map, meta_blk);
        break;
      }

      if (meta_blk->tail_recursion) {
        blk->recursion_map = obj_map;
        break;
      }

      if (meta_blk->error_returning || 
        meta_blk->error_obj_checking ||
        meta_blk->error_assembling) break;

      msg = ent.Start(obj_map);
      msg.Get(&value, &code, &detail);

      if (value == kStrFatalError) break;

      if (code == kCodeObject && ent.GetTokenEnum() != GT_TYPE_ASSERT) {
        auto object = msg.GetObj();

        if (is_operator_token && idx + 1 < action_base.size()) {
          entry::IsOperatorToken(action_base[idx + 1].first.GetTokenEnum()) ?
            meta_blk->returning_base.emplace_front(object) :
            meta_blk->returning_base.emplace_back(object);
        }
        else {
          meta_blk->returning_base.emplace_back(object);
        }
      }
    }

    if (meta_blk->error_returning || 
      meta_blk->error_obj_checking ||
      meta_blk->error_assembling) {
      msg = Message(kStrFatalError, kCodeIllegalSymbol, meta_blk->error_string);
    }

    if(!preprocessing && meta_blk->tail_recursion) 
      blk->tail_recursion = meta_blk->tail_recursion;

    obj_map.clear();
    meta_blk->returning_base.clear();
    meta_blk->returning_base.shrink_to_fit();

    delete meta_blk;

    return msg;
  }

  Message Machine::PreProcessing() {
    Meta *meta = nullptr;
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
      meta = &storage_[idx];
      token = entry::GetGenericToken(meta->GetMainToken().first);
      if (token == GT_WHILE || token == GT_IF || token == GT_CASE) {
        nest_head_count++;
      }
      else if (token == GT_DEF) {
        if (flag == true) {
          result = Message(kStrFatalError, kCodeBadExpression, "Define function in function is not supported.").SetIndex(idx);
          break;
        }
        result = MetaProcessing(*meta, "", nullptr);
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

    vector<Meta> *otherMeta = new vector<Meta>();
    bool filter = false;
    for (size_t idx = 0; idx < storage_.size(); ++idx) {
      auto it = skipped_idx.find(idx);
      if (it != skipped_idx.end()) {
        idx = it->second;
        continue;
      }
      otherMeta->emplace_back(storage_[idx]);
    }

    storage_.swap(*otherMeta);
    delete otherMeta;

    return result;
  }

  void Machine::InitGlobalObject(bool create_container, string name) {
    if (create_container) entry::CreateContainer();

    auto create = [&](string id, string value)->void {
      entry::CreateObject(id, Object()
        .Manage("'" + value + "'",T_STRING));
    };

    if (is_main_) {
      create("__name__", "__main__");
    }
    else {
      if (name != kStrEmpty) {
        create("__name__", name);
      }
      else {
        create("__name__", "");
      }
    }
  }



  bool Machine::PredefinedMessage(Message &result, size_t mode, Token token) {
    bool judged = false;
    GenericTokenEnum gen_token = entry::GetGenericToken(token.first);

    switch (mode) {
    case kModeNextCondition:
      if (entry::HasTailTokenRequest(gen_token)) {
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
      if (entry::HasTailTokenRequest(gen_token)) {
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

  void Machine::TailRecursionActions(MachCtlBlk *blk,string &name) {
    Object obj = *entry::FindObject(kStrUserFunc);
    auto &base = entry::GetCurrentContainer();
    base.clear();
    base.Add(kStrUserFunc, obj);
    for (auto &unit : blk->recursion_map) {
      base.Add(unit.first, unit.second);
    }
    blk->recursion_map.clear();
    ResetBlock(blk);
    ResetContainer(name);
  }

  Message Machine::Run(bool create_container, string name) {
    Message result;
    MachCtlBlk *blk = new MachCtlBlk();
    Meta *meta = nullptr;
    bool judged = false;

    ResetBlock(blk);
    health_ = true;

    if (storage_.empty()) return result;

    InitGlobalObject(create_container, name);
    if (result.GetCode() < kCodeSuccess) return result;

    //Main state machine
    while (blk->current < storage_.size()) {
      if (!health_) break;

      blk->last_index = (blk->current == storage_.size() - 1);
      meta = &storage_[blk->current];

      judged = PredefinedMessage(result, blk->mode, meta->GetMainToken());
      
      if (!judged) result = MetaProcessing(*meta, name, blk);

      const auto value = result.GetValue();
      const auto code  = result.GetCode();
      const auto detail = result.GetDetail();

      if (blk->tail_recursion) {
        TailRecursionActions(blk, name);
        continue;
      }

      if (value == kStrFatalError) {
        trace::Log(result.SetIndex(storage_[blk->current].GetIndex()));
        break;
      }

      if (value == kStrWarning) {
        trace::Log(result.SetIndex(storage_[blk->current].GetIndex()));
      }

      if (value == kStrStopSign) {
        break;
      }

      switch (code) {
      case kCodeContinue:
        Continue(blk);
        break;
      case kCodeBreak:
        Break(blk);
        break;
      case kCodeConditionRoot:
        ConditionRoot(GetBooleanValue(value), blk); 
        break;
      case kCodeConditionBranch:
        if (blk->nest_head_count > 0) break;
        ConditionBranch(GetBooleanValue(value), blk);
        break;
      case kCodeConditionLeaf:
        if (blk->nest_head_count > 0) break;
        ConditionLeaf(blk);
        break;
      case kCodeCase:
        CaseHead(result, blk);
        break;
      case kCodeWhen:
        WhenHead(GetBooleanValue(value), blk);
        break;
      case kCodeHeadSign:
        HeadSign(GetBooleanValue(value), blk);
        break;
      case kCodeHeadPlaceholder:
        blk->nest_head_count++;
        break;
      case kCodeTailSign:
        if (blk->nest_head_count > 0) {
          blk->nest_head_count--;
          break;
        }
        TailSign(blk);
        break;
      default:break;
      }

      if (blk->runtime_error) break;

      ++blk->current;
      if (judged) judged = false;
    }

    if (blk->runtime_error) {
      trace::Log(Message(kStrFatalError, kCodeBadExpression, 
        blk->error_string).SetIndex(blk->current));
    }

    if (create_container) entry::DisposeManager();

    delete blk;
    return result;
  }

  Message Machine::RunAsFunction(ObjectMap &p) {
    Message msg;
    auto &base = entry::CreateContainer();
    string func_id = p.Get<string>(kStrUserFunc);

    for (auto &unit : p) {
      base.Add(unit.first, unit.second);
    }

    msg = Run(false, func_id);

    if (msg.GetCode() >= kCodeSuccess) {
      msg = Message();
      auto &currentBase = entry::GetCurrentContainer();
      Object *ret = currentBase.Find(kStrRetValue);
      if (ret != nullptr) {
        Object obj;
        obj.Copy(*ret);
        msg.SetObject(obj);
      }
    }

    Object *func_sign = entry::GetCurrentContainer().Find(kStrUserFunc);

    ResetContainer(func_id);

    entry::DisposeManager();
    return msg;
  }

  Machine::Machine(const char *target, bool is_main) {
    std::wifstream stream;
    wstring buf;
    health_ = true;
    size_t subscript = 0;
    vector<string> script_buf;

    is_main_ = is_main;

    stream.open(target, std::ios::in);
    if (stream.good()) {
      while (!stream.eof()) {
        std::getline(stream, buf);
        string temp = ws2s(buf);
        if (!temp.empty() && temp.back() == '\0') temp.pop_back();
        script_buf.emplace_back(temp);
      }
    }
    stream.close();

    vector<StringUnit> string_units = MultilineProcessing(script_buf);
    Analyzer analyzer;
    Message msg;
    for (auto it = string_units.begin(); it != string_units.end(); ++it) {
      if (it->second == "") continue;
      msg = analyzer.Make(it->second, it->first);
      if (msg.GetValue() == kStrFatalError) {
        trace::Log(msg.SetIndex(subscript));
        health_ = false;
        break;
      }
      if (msg.GetValue() == kStrWarning) {
        trace::Log(msg.SetIndex(subscript));
      }
      storage_.emplace_back(Meta(
        analyzer.GetOutput(),
        analyzer.get_index(),
        analyzer.GetMainToken()));
      analyzer.Clear();
    }

    msg = PreProcessing();
    if (msg.GetValue() == kStrFatalError) {
      health_ = false;
      trace::Log(msg);
    }
  }

  void Machine::Reset(MachCtlBlk *blk) {
    while (!blk->cycle_nest.empty()) blk->cycle_nest.pop();
    while (!blk->cycle_tail.empty()) blk->cycle_tail.pop();
    while (!blk->mode_stack.empty()) blk->mode_stack.pop();
    while (!blk->condition_stack.empty()) blk->condition_stack.pop();
    delete blk;
  }
}

