//#include <iostream>
#include <ctime>
#ifndef _NO_CUI_
#include <iostream>
#endif
#include "machine.h"

namespace kagami {
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

  string IndentationAndCommentProc(string target) {
    if (target == "") return "";
    string data;
    char currentChar, forwardChar;
    size_t head = 0, tail = 0;
    bool exemptBlankChar = true;
    bool stringProcessing = false;
    auto toString = [](char t) ->string {return string().append(1, t); };

    for (size_t count = 0; count < target.size(); ++count) {
      currentChar = target[count];
      auto type = kagami::Kit::GetTokenType(toString(currentChar));
      if (type != TokenTypeEnum::T_BLANK && exemptBlankChar) {
        head = count;
        exemptBlankChar = false;
      }
      if (currentChar == '\'' && forwardChar != '\\') stringProcessing = !stringProcessing;
      if (!stringProcessing && currentChar == '#') {
        tail = count;
        break;
      }
      forwardChar = target[count];
    }
    if (tail > head) data = target.substr(head, tail - head);
    else data = target.substr(head, target.size() - head);
    if (data.front() == '#') return "";

    while (!data.empty() &&
      Kit::GetTokenType(toString(data.back())) == TokenTypeEnum::T_BLANK) {
      data.pop_back();
    }
    return data;
  }

  vector<StringUnit> MultilineProcessing(vector<string> &src) {
    vector<StringUnit> output;
    string buf;
    size_t idx = 0, lineIdx = 0;

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
      lineIdx = idx;
      while (buf.back() == '_') {
        bool inString = isInString(buf);
        if (!inString) {
          idx += 1;
          buf.pop_back();
          buf = buf + IndentationAndCommentProc(src[idx]);
        }
      }
      output.push_back(StringUnit(lineIdx, buf));
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
    base[id] = Machine(proc).SetParameters(parms);
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
    Object &funcId = p[kStrUserFunc];
    string id = *static_pointer_cast<string>(funcId.Get());
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
    vector<string> parms = Kit::BuildStringVector(parmStr);
    ObjectMap objMap;
    for (size_t i = 0; i < parms.size(); i++) {
      objMap.insert(NamedObject(parms[i], objects[i]));
    }
    return activity(objMap);
  }

  void Machine::MakeFunction(size_t start,size_t end, vector<string> &defHead) {
    if (start > end) return;
    string id = defHead[0];
    vector<string> parms;
    vector<Meta> proc;

    for (size_t i = 1; i < defHead.size(); ++i) {
      parms.push_back(defHead[i]);
    }
    for (size_t j = start; j <= end; ++j) {
      proc.push_back(storage[j]);
    }
    AddFunction(id, proc, parms);
  }

  Machine &Machine::SetParameters(vector<string> parms) {
    this->parameters = parms;
    return *this;
  }

  void Machine::Reset(MachCtlBlk *blk) {
    Kit().CleanupVector(storage);
    while (!blk->cycleNestStack.empty()) blk->cycleNestStack.pop();
    while (!blk->cycleTailStack.empty()) blk->cycleTailStack.pop();
    while (!blk->modeStack.empty()) blk->modeStack.pop();
    while (!blk->conditionStack.empty()) blk->conditionStack.pop();
    delete blk;
  }

  Machine::Machine(const char *target, bool isMain) {
    std::wifstream stream;
    wstring buf;
    health = true;
    size_t subscript = 0;
    vector<string> scriptBuf;

    this->isMain = isMain;

    stream.open(target, std::ios::in);
    if (stream.good()) {
      while (!stream.eof()) {
        std::getline(stream, buf);
        string temp = ws2s(buf);
        if (!temp.empty() && temp.back() == '\0') temp.pop_back();
        scriptBuf.emplace_back(temp);
      }
    }
    stream.close();

    vector<StringUnit> stringUnit = MultilineProcessing(scriptBuf);
    Analyzer analyzer;
    Message msg;
    for (auto it = stringUnit.begin(); it != stringUnit.end(); ++it) {
      if (it->second == "") continue;
      msg = analyzer.Make(it->second, it->first);
      if (msg.GetValue() == kStrFatalError) {
        trace::Log(msg.SetIndex(subscript));
        health = false;
        break;
      }
      if (msg.GetValue() == kStrWarning) {
        trace::Log(msg.SetIndex(subscript));
      }
      storage.emplace_back(Meta(
        analyzer.GetOutput(),
        analyzer.GetIdx(),
        analyzer.GetMainToken()));
      analyzer.Clear();
    }

    msg = PreProcessing();
    if (msg.GetValue() == kStrFatalError) {
      health = false;
      trace::Log(msg);
    }
  }

  void Machine::CaseHead(Message &msg, MachCtlBlk *blk) {
    entry::CreateContainer();
    blk->modeStack.push(blk->currentMode);
    blk->currentMode = kModeCaseJump;
    blk->conditionStack.push(false);
  }

  void Machine::WhenHead(bool value, MachCtlBlk *blk) {
    if (!blk->conditionStack.empty()) {
      if (blk->currentMode == kModeCase && blk->conditionStack.top() == true) {
        blk->currentMode = kModeCaseJump;
      }
      else if (value == true && blk->conditionStack.top() == false) {
        blk->currentMode = kModeCase;
        blk->conditionStack.top() = true;
      }
    }
  }

  void Machine::ConditionRoot(bool value, MachCtlBlk *blk) {
    blk->modeStack.push(blk->currentMode);
    entry::CreateContainer();
    if (value == true) {
      blk->currentMode = kModeCondition;
      blk->conditionStack.push(true);
    }
    else {
      blk->currentMode = kModeNextCondition;
      blk->conditionStack.push(false);
    }
  }

  void Machine::ConditionBranch(bool value, MachCtlBlk *blk) {
    if (!blk->conditionStack.empty()) {
      if (blk->conditionStack.top() == false && blk->currentMode == kModeNextCondition
        && value == true) {
        entry::CreateContainer();
        blk->currentMode = kModeCondition;
        blk->conditionStack.top() = true;
      }
      else if (blk->conditionStack.top() == true && blk->currentMode == kModeCondition) {
        blk->currentMode = kModeNextCondition;
      }
    }
    else {
      //msg.combo
      health = false;
    }
  }

  void Machine::ConditionLeaf(MachCtlBlk *blk) {
    if (!blk->conditionStack.empty()) {
      if (blk->conditionStack.top() == true) {
        switch (blk->currentMode) {
        case kModeCondition:
        case kModeNextCondition:
          blk->currentMode = kModeNextCondition;
          break;
        case kModeCase:
        case kModeCaseJump:
          blk->currentMode = kModeCaseJump;
          break;
        }
      }
      else {
        entry::CreateContainer();
        blk->conditionStack.top() = true;
        switch (blk->currentMode) {
        case kModeNextCondition:
          blk->currentMode = kModeCondition;
          break;
        case kModeCaseJump:
          blk->currentMode = kModeCase;
          break;
        }
      }
    }
    else {
      //msg.combo
      health = false;
    }
  }

  void Machine::HeadSign(bool value, MachCtlBlk *blk) {
    if (blk->cycleNestStack.empty()) {
      blk->modeStack.push(blk->currentMode);
      entry::CreateContainer();
    }
    else {
      if (blk->cycleNestStack.top() != blk->current - 1) {
        blk->modeStack.push(blk->currentMode);
        entry::CreateContainer();
      }
    }
    if (value == true) {
      blk->currentMode = kModeCycle;
      if (blk->cycleNestStack.empty()) {
        blk->cycleNestStack.push(blk->current - 1);
      }
      else if (blk->cycleNestStack.top() != blk->current - 1) {
        blk->cycleNestStack.push(blk->current - 1);
      }
    }
    else if (value == false) {
      blk->currentMode = kModeCycleJump;
      if (!blk->cycleTailStack.empty()) {
        blk->current = blk->cycleTailStack.top();
      }
    }
    else {
      health = false;
    }
  }

  void Machine::TailSign(MachCtlBlk *blk) {
    if (blk->currentMode == kModeCondition || blk->currentMode == kModeNextCondition) {
      blk->conditionStack.pop();
      blk->currentMode = blk->modeStack.top();
      blk->modeStack.pop();
      entry::DisposeManager();
    }
    else if (blk->currentMode == kModeCycle || blk->currentMode == kModeCycleJump) {
      switch (blk->currentMode) {
      case kModeCycle:
        if (blk->cycleTailStack.empty() || blk->cycleTailStack.top() != blk->current - 1) {
          blk->cycleTailStack.push(blk->current - 1);
        }
        blk->current = blk->cycleNestStack.top();
        entry::GetCurrentContainer().clear();
        break;
      case kModeCycleJump:
        if (blk->sContinue) {
          if (blk->cycleTailStack.empty() || blk->cycleTailStack.top() != blk->current - 1) {
            blk->cycleTailStack.push(blk->current - 1);
          }
          blk->current = blk->cycleNestStack.top();
          blk->currentMode = kModeCycle;
          blk->modeStack.top() = blk->currentMode;
          blk->sContinue = false;
          entry::GetCurrentContainer().clear();
        }
        else {
          if (blk->sBreak) blk->sBreak = false;
          blk->currentMode = blk->modeStack.top();
          blk->modeStack.pop();
          if (!blk->cycleNestStack.empty()) blk->cycleNestStack.pop();
          if (!blk->cycleTailStack.empty()) blk->cycleTailStack.pop();
          entry::DisposeManager();
        }
        break;
      default:break;
      }
    }
    else if (blk->currentMode == kModeCase || blk->currentMode == kModeCaseJump) {
      blk->conditionStack.pop();
      blk->currentMode = blk->modeStack.top();
      blk->modeStack.pop();
      entry::DisposeManager();
    }
  }

  void Machine::Continue(MachCtlBlk *blk) {
    while (!blk->modeStack.empty() && blk->currentMode != kModeCycle) {
      if (blk->currentMode == kModeCondition) {
        blk->conditionStack.pop();
        blk->nestHeadCount++;
      }
      blk->currentMode = blk->modeStack.top();
      blk->modeStack.pop();
    }
    blk->currentMode = kModeCycleJump;
    blk->sContinue = true;
  }

  void Machine::Break(MachCtlBlk *blk) {
    while (!blk->modeStack.empty() && blk->currentMode != kModeCycle) {
      if (blk->currentMode == kModeCondition) {
        blk->conditionStack.pop();
        blk->nestHeadCount++;
      }
      blk->currentMode = blk->modeStack.top();
      blk->modeStack.pop();
    }
    if (blk->currentMode == kModeCycle) {
      blk->currentMode = kModeCycleJump;
      blk->sBreak = true;
    }
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

  Message Machine::MetaProcessing(Meta &meta) {
    Kit kit;
    int mode, flag;
    string errorString, id, typeId, vaArgHead;
    Message msg;
    ObjectMap objMap;
    deque<Object> retBase;
    vector<string> args;
    size_t sub = 0;
    size_t count = 0;
    size_t vaArgSize = 0;
    vector<Instruction> &actionBase = meta.GetContains();
    bool errorReturn = false, errorArg = false;

    auto makeObject = [&](Parameter &parm) -> Object {
      Object obj;
      ObjectPointer ptr;
      switch (parm.type) {
      case PT_NORMAL:
        obj.Manage(parm.data)
          .SetMethods(type::GetMethods(kTypeIdRawString))
          .SetTokenType(parm.tokenType);
        break;
      case PT_OBJ:
        ptr = entry::FindObject(parm.data);
        if (ptr != nullptr) {
          obj.Ref(*ptr);
        }
        else {
          errorString = "Object is not found - " + parm.data;
          errorArg = true;
        }
        break;
      case PT_RET:
        if (!retBase.empty()) {
          obj = retBase.front();
          retBase.pop_front();
        }
        else {
          errorString = "Return base error.";
          errorReturn = true;
        }
        break;
      default:
        break;
      }

      return obj;
    };

    for (size_t idx = 0; idx < actionBase.size(); idx += 1) {
      kit.CleanupMap(objMap).CleanupVector(args);
      id.clear();
      vaArgHead.clear();
      typeId.clear();
      sub = 0;
      count = 0;

      auto &ent = actionBase[idx].first;
      auto &parms = actionBase[idx].second;
      auto entParmSize = ent.GetParmSize();

      if (ent.GetEntrySign()) {
        id = ent.GetId();
        ent.NeedSpecificType() ?
          typeId = makeObject(parms.back()).GetTypeId() :
          typeId = kTypeIdNull;

        ent = entry::Order(id, typeId);

        if (!ent.Good()) {
          msg.combo(kStrFatalError, kCodeIllegalCall, "Function not found - " + id);
          break;
        }
      }

      args = ent.GetArguments();
      mode = ent.GetArgumentMode();
      flag = ent.GetFlag();

      if (mode == kCodeAutoSize) {
        while (sub < entParmSize - 1) {
          objMap.insert(NamedObject(args[sub], makeObject(parms[sub])));
          sub += 1;
        }

        vaArgHead = args.back();
        flag == kFlagMethod ?
          vaArgSize = parms.size() - 1 :
          vaArgSize = parms.size();

        while (sub < vaArgSize) {
          objMap.insert(NamedObject(vaArgHead + to_string(count), makeObject(parms[sub])));
          count += 1;
          sub += 1;
        }

        objMap.insert(NamedObject("__size",
          Object().Manage(to_string(count))
          .SetMethods(type::GetMethods(kTypeIdRawString))
          .SetTokenType(T_INTEGER)));
      }
      else {
        while (sub < args.size()) {
          if (sub >= parms.size() && mode == kCodeAutoFill) break;
          //error
          objMap.insert(NamedObject(args[sub], makeObject(parms[sub])));
          sub += 1;
        }
      }

      if (flag == kFlagMethod) {
        objMap.insert(NamedObject(kStrObject, makeObject(parms.back())));
      }

      if (errorReturn || errorArg) break;
      msg = ent.Start(objMap);
      const auto code = msg.GetCode();
      const auto value = msg.GetValue();
      const auto detail = msg.GetDetail();

      if (value == kStrFatalError) break;

      if (code == kCodeObject) {
        auto object = msg.GetObj();
        retBase.emplace_back(object);
      }
      else if ((value == kStrRedirect && (code == kCodeSuccess || code == kCodeHeadPlaceholder))
        && ent.GetTokenEnum() != GT_TYPE_ASSERT) {
        Object obj;
        obj.Manage(detail)
          .SetMethods(type::GetMethods(kTypeIdRawString))
          .SetTokenType(kagami::Kit::GetTokenType(detail));
        if (entry::IsOperatorToken(ent.GetTokenEnum())
          && idx + 1 < actionBase.size()) {
          auto token = actionBase[idx + 1].first.GetTokenEnum();
          if (entry::IsOperatorToken(token)) {
            retBase.emplace_front(obj);
          }
          else {
            retBase.emplace_back(obj);
          }
        }
        else {
          retBase.emplace_back(obj);
        }
      }
    }

    if (errorReturn || errorArg) {
      msg.combo(kStrFatalError, kCodeIllegalSymbol, errorString);
    }

    return msg;
  }

  Message Machine::PreProcessing() {
    Meta *meta = nullptr;
    GenericTokenEnum token;
    Message result;
    bool flag = false;
    map<size_t, size_t> skippedIndex;
    using IndexPair = pair<size_t, size_t>;
    size_t nestHeadCount = 0;
    size_t defStart;
    vector<string> defHead;

    for (size_t idx = 0; idx < storage.size(); ++idx) {
      if (!health) break;
      meta = &storage[idx];
      token = entry::GetGenericToken(meta->GetMainToken().first);
      if (token == GT_WHILE || token == GT_IF || token == GT_CASE) {
        nestHeadCount++;
      }
      else if (token == GT_DEF) {
        if (flag == true) {
          result.combo(kStrFatalError, kCodeBadExpression, "Define function in function is not supported.").SetIndex(idx);
          break;
        }
        result = MetaProcessing(*meta);
        defHead = Kit().BuildStringVector(result.GetDetail());
        defStart = idx + 1;
        flag = true;
      }
      else if (token == GT_END) {
        if (nestHeadCount > 0) {
          nestHeadCount--;
        }
        else {
          skippedIndex.insert(IndexPair(defStart - 1, idx));
          MakeFunction(defStart, idx - 1, defHead);
          Kit().CleanupVector(defHead);
          defStart = 0;
          flag = false;
        }
      }
    }

    vector<Meta> *otherMeta = new vector<Meta>();
    bool filter = false;
    for (size_t idx = 0; idx < storage.size(); ++idx) {
      auto it = skippedIndex.find(idx);
      if (it != skippedIndex.end()) {
        idx = it->second;
        continue;
      }
      otherMeta->emplace_back(storage[idx]);
    }

    storage.swap(*otherMeta);
    delete otherMeta;

    return result;
  }

  void Machine::InitGlobalObject(bool createContainer) {
    if (createContainer) entry::CreateContainer();
    if (isMain) {
      entry::CreateObject("__name__", Object()
        .Manage("'__main__'")
        .SetMethods(type::GetMethods(kTypeIdRawString)));
    }
    else {
      //TODO:module name
      entry::CreateObject("__name__", Object()
        .Manage("''")
        .SetMethods(type::GetMethods(kTypeIdRawString)));
    }
  }

  Message Machine::Run(bool createContainer) {
    Message result;
    MachCtlBlk *blk = new MachCtlBlk();
    Meta *meta = nullptr;
    GenericTokenEnum token;
    bool judged = false;
    blk->currentMode = kModeNormal;
    blk->nestHeadCount = 0;
    blk->current = 0;
    blk->currentMode = kModeNormal;
    blk->defStart = 0;
    blk->sContinue = false;
    blk->sBreak = false;
    health = true;

    if (storage.empty()) return result;

    InitGlobalObject(createContainer);
    if (result.GetCode() < kCodeSuccess) return result;

    //Main state machine
    while (blk->current < storage.size()) {
      if (!health) break;
      meta = &storage[blk->current];

      token = entry::GetGenericToken(meta->GetMainToken().first);
      switch (blk->currentMode) {
      case kModeNextCondition:
        if (entry::HasTailTokenRequest(token)) {
          result.combo(kStrRedirect, kCodeHeadPlaceholder, kStrTrue);
          judged = true;
        }
        else if (token != GT_ELSE && token != GT_END && token != GT_ELIF) {
          result.combo(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
          judged = true;
        }
        break;
      case kModeCycleJump:
        if (token != GT_END && token != GT_IF && token != GT_WHILE) {
          result.combo(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
          judged = true;
        }
        break;
      case kModeCaseJump:
        if (entry::HasTailTokenRequest(token)) {
          result.combo(kStrRedirect, kCodeHeadPlaceholder, kStrTrue);
          judged = true;
        }
        else if (token != GT_WHEN && token != GT_END) {
          result.combo(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
          judged = true;
        }
        break;
      default:
        break;
      }
      
      if (!judged) result = MetaProcessing(*meta);

      const auto value = result.GetValue();
      const auto code  = result.GetCode();

      if (value == kStrFatalError) {
        trace::Log(result.SetIndex(storage[blk->current].GetIndex()));
        break;
      }

      if (value == kStrWarning) {
        trace::Log(result.SetIndex(storage[blk->current].GetIndex()));
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
        if (blk->nestHeadCount > 0) break;
        ConditionBranch(GetBooleanValue(value), blk);
        break;
      case kCodeConditionLeaf:
        if (blk->nestHeadCount > 0) break;
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
        blk->nestHeadCount++;
        break;
      case kCodeTailSign:
        if (blk->nestHeadCount > 0) {
          blk->nestHeadCount--;
          break;
        }
        TailSign(blk);
        break;
      default:break;
      }
      ++blk->current;
      if (judged) judged = false;
    }

    if (createContainer) entry::DisposeManager();
    delete blk;
    return result;
  }

  Message Machine::RunAsFunction(ObjectMap &p) {
    Message msg;
    auto &base = entry::CreateContainer();
    for (auto &unit : p) {
      base.Add(unit.first, unit.second);
    }
    msg = Run(false);
    if (msg.GetCode() >= kCodeSuccess) {
      msg = Message();
      Object *ret = base.Find(kStrRetValue);
      if (ret != nullptr) {
        Object obj;
        obj.Copy(*ret);
        msg.SetObject(obj, "__result");
      }
    }
    Object *funcSign = entry::GetCurrentContainer().Find(kStrUserFunc);
    string funcId = *static_pointer_cast<string>(p[kStrUserFunc].Get());
    while (funcSign == nullptr) {
      entry::DisposeManager();
      funcSign = entry::GetCurrentContainer().Find(kStrUserFunc);
      if (funcSign != nullptr) {
        string value = *static_pointer_cast<string>(p[kStrUserFunc].Get());
        if (value == funcId) break;
      }
    }
    entry::DisposeManager();
    return msg;
  }
}

