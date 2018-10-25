#include "component.h"

namespace kagami {
  GroupTypeEnum GetGroupType(Object &A, Object &B) {
    auto data_A = GetObjectStuff<string>(A),
      data_B = GetObjectStuff<string>(B);
    auto data_type_A = A.GetTokenType();
    auto data_type_B = B.GetTokenType();

    GroupTypeEnum group_type = GroupTypeEnum::G_NUL;

    if (data_type_A == T_FLOAT || data_type_B == T_FLOAT) {
      group_type = G_FLOAT;
    }
    if (data_type_A == T_INTEGER && data_type_B == T_INTEGER) {
      group_type = G_INT;
    }
    if (util::IsString(data_A) || util::IsString(data_B)) {
      group_type = G_STR;
    }
    if ((data_A == kStrTrue || data_A == kStrFalse) &&
      (data_B == kStrTrue || data_B == kStrFalse)) {
      group_type = G_STR;
    }

    return group_type;
  }

  inline bool IsStringObject(Object &obj) {
    auto id = obj.GetTypeId();
    return (id == kTypeIdRawString || id == kTypeIdString);
  }

  inline bool CheckObjectType(Object &obj, string type_id) {
    return (obj.GetTypeId() == type_id);
  }

  inline bool CheckTokenType(Object &obj, TokenTypeEnum tokenType) {
    return (obj.GetTokenType() == tokenType);
  }

  inline bool IsRawStringObject(Object &obj) {
    return CheckObjectType(obj, kTypeIdRawString);
  }

  inline Message IllegalCallMsg(string str) {
    return Message(kStrFatalError, kCodeIllegalCall, str);
  }

  inline Message IllegalParmMsg(string str) {
    return Message(kStrFatalError, kCodeIllegalParm, str);
  }

  inline Message IllegalSymbolMsg(string str) {
    return Message(kStrFatalError, kCodeIllegalSymbol, str);
  }

  inline void CopyObject(Object &dest, Object &src) {
    dest.Set(type::GetObjectCopy(src), src.GetTypeId(),
      src.GetMethods(), false)
      .SetTokenType(src.GetTokenType());
  }

  inline Message CheckEntryAndStart(string id, string type_id, ObjectMap &parm) {
    Message msg;
    auto ent = entry::Order(id, type_id);
    ent.Good() ?
      msg = ent.Start(parm) :
      msg.SetCode(kCodeIllegalCall);
    return msg;
  }

  Message Define(ObjectMap &p) {
    vector<string> def_head;
    size_t count = 0;
    def_head.emplace_back(p.Get<string>("id"));

    for (auto &unit : p) {
      if (unit.first == "arg" + to_string(count)) {
        string str = GetObjectStuff<string>(unit.second);
        def_head.emplace_back(str);
        count++;
      }
    }
    string def_head_string = util::CombineStringVector(def_head);
    return Message(kStrEmpty, kCodeDefineSign, def_head_string);
  }

  Message ReturnSign(ObjectMap &p) {
    auto &container = entry::GetCurrentContainer();
    if (p.Search("value")) {
      auto &obj = p["value"];
      container.Add(kStrRetValue, 
        Object(obj.Get(), obj.GetTypeId(), obj.GetMethods(), false));
    }
    else {
      container.Add(kStrRetValue, Object());
    }

    return Message(kStrStopSign, kCodeSuccess, kStrEmpty);
  }

  Message WriteLog(ObjectMap &p) {
    CONDITION_ASSERT(IsStringObject(p["msg"]), "Illegal output string.");

    Message result;
    ofstream ofs("kagami-script.log", std::ios::out | std::ios::app);
    string str = p.Get<string>("msg");

    ofs << RealString(str) << "\n";
    ofs.close();
    return result;
  }

  string IncAndDecOperation(Object &obj, bool negative, bool keep) {
    string res, origin;

    if (CheckObjectType(obj, kTypeIdRawString)) {
      origin = GetObjectStuff<string>(obj);
      if (CheckTokenType(obj, T_INTEGER)) {
        int data = stoi(origin);
        negative ?
          data -= 1 :
          data += 1;
        keep ?
          res = origin :
          res = to_string(data);
        obj.Copy(MakeObject(data));
      }
      else {
        res = origin;
      }
    }

    return res;
  }

  inline void CheckSelfOperatorMsg(Message &msg, string res) {
    res.empty() ?
      msg = Message(res) :
      msg = IllegalParmMsg("Illegal self-operator.");
  }

  Message GetRawStringType(ObjectMap &p) {
    OBJECT_ASSERT(p, "object", kTypeIdRawString);

    string result;
    string str = RealString(p.Get<string>("object"));

    switch (util::GetTokenType(str)) {
    case T_BOOLEAN:result = "'boolean'"; break;
    case T_GENERIC:result = "'generic'"; break;
    case T_INTEGER:result = "'integer'"; break;
    case T_FLOAT:result = "'float'"; break;
    case T_SYMBOL:result = "'symbol'"; break;
    case T_BLANK:result = "'blank'"; break;
    case T_STRING:result = "'string'"; break;
    case T_NUL:result = "'null'"; break;
    default:result = "'null'"; break;
    }

    return Message(result);
  }

  Message GetTypeId(ObjectMap &p) {
    return Message(p["object"].GetTypeId());
  }

  Message BindAndSet(ObjectMap &p) {
    Object &dest = p["object"], source = p["source"];
    Message msg;

    if (dest.IsRef()) {
      CONDITION_ASSERT(!dest.get_ro(), "Object is read-only.");
      CopyObject(dest, source);
    }
    else {
      string id = GetObjectStuff<string>(dest);
      CONDITION_ASSERT(util::GetTokenType(id) == T_GENERIC, 
        "Illegal bind operation.");

      //TODO:Optimize for domain
      ObjectPointer real_dest = entry::FindObject(id);
      if (real_dest != nullptr) {
        CopyObject(*real_dest, source);
      }
      else {
        Object base(type::GetObjectCopy(source), source.GetTypeId(),
          source.GetMethods(), false);
        base.SetTokenType(source.GetTokenType());

        auto result = entry::CreateObject(id, base);

        CALL_ASSERT(result != nullptr, "Object creation failed.");
      }
    }

    return msg;
  }

  Message Print(ObjectMap &p) {
    Object &obj = p[kStrObject];

    auto errorMsg = []() {
      std::cout << "You can't print this object." << std::endl;
    };

    if (!util::FindInStringGroup("__print", obj.GetMethods())) {
      errorMsg();
    } 
    else {
      Message tempMsg = CheckEntryAndStart("__print", obj.GetTypeId(), p);
      if (tempMsg.GetCode() == kCodeIllegalCall) errorMsg();
    }
    return Message();
  }

  Message GetTimeDate(ObjectMap &p) {
    auto now = time(nullptr);
#if defined(_WIN32) && defined(_MSC_VER)
    char nowTime[30] = { ' ' };
    ctime_s(nowTime, sizeof(nowTime), &now);
    string str(nowTime);
    str.pop_back(); //erase '\n'
    return Message("'" + str + "'");
#else
    string TimeData(ctime(&now));
    return Message("'" + TimeData + "'");
#endif
  }

  Message Input(ObjectMap &p) {
    if (p.Search("msg")) {
      CONDITION_ASSERT(IsStringFamily(p["msg"]), 
        "Illegal message string.");
      
      ObjectMap obj_map;
      obj_map.Input("not_wrap");
      obj_map.Input(kStrObject, p["msg"]);
      Print(obj_map);
    }

    string buf;
    std::getline(std::cin, buf);
    return Message("'" + buf + "'");
  }

  Message Convert(ObjectMap &p) {
    OBJECT_ASSERT(p, "object", kTypeIdRawString);

    string origin = RealString(p.Get<string>("object"));
    auto type = util::GetTokenType(origin);
    string str;

    (type == T_NUL || type == T_GENERIC) ?
      str = kStrNull :
      str = origin;
    
    return Message().SetObject(Object(str, type));
  }

  Message Quit(ObjectMap &p) {
    return Message(kStrEmpty, kCodeQuit, kStrEmpty);
  }

  Message Nop(ObjectMap &p) {
    int size = p.GetVaSize();
    Object &last_obj = p("nop", size - 1);
    return Message().SetObject(last_obj);
  }

  Message ArrayMaker(ObjectMap &p) {
    vector<Object> base;
    Message msg;
    int size = p.GetVaSize();

    if (!p.empty()) {
      for (int i = 0; i < size; i++) {
        base.emplace_back(p("item", i));
      }
    }

    msg.SetObject(Object(make_shared<vector<Object>>(base), kTypeIdArrayBase,
      type::GetMethods(kTypeIdArrayBase), false)
      .SetConstructorFlag());

    return msg;
  }

  Message Dir(ObjectMap &p) {
    Object &obj = p["object"];
    auto vec = util::BuildStringVector(obj.GetMethods());
    Message msg;
    vector<Object> output;

    for (auto &unit : vec) {
      output.emplace_back(Object(make_shared<string>(unit), kTypeIdString, 
          type::GetMethods(kTypeIdString), true));
    }

    msg.SetObject(Object(make_shared<vector<Object>>(output),kTypeIdArrayBase, 
        kArrayBaseMethods, true)
      .SetConstructorFlag());

    return msg;
  }


  Message Exist(ObjectMap &p){
    Object &obj = p["object"];
    string target = RealString(p.Get<string>("id"));
    bool result = util::FindInStringGroup(target, obj.GetMethods());
    Message msg;
    result ?
      msg = Message(kStrTrue) :
      msg = Message(kStrFalse);
    return msg;
  }

  Message TypeAssert(ObjectMap &p) {
    Object &obj = p["object"];
    string target = p.Get<string>("id");
    bool result = util::FindInStringGroup(target, obj.GetMethods());
    Message msg;

    CALL_ASSERT(result, "Method not found - " + target);

    Object *ret = entry::FindObject(target, obj.GetTypeId());
    if (ret != nullptr) {
      msg.SetObject(*ret);
    }
    else {
      Object obj_ent;
      auto ent = entry::Order(target, obj.GetTypeId());
      if (ent.Good()) {
        obj_ent.Set(make_shared<Entry>(ent), kTypeIdFunction, 
          kFunctionMethods, false);
        msg.SetObject(obj_ent);
      }
    }
   
    return msg;
  }

  Message Case(ObjectMap &p) {
    Object &obj = p["object"];
    auto copy = type::GetObjectCopy(obj);

    //TODO:Re-design
    CONDITION_ASSERT(IsStringObject(obj), 
      "Case-When is not supported yet.(01)");
      
    Object base(copy, obj.GetTypeId(), obj.GetMethods(), false);
    entry::CreateObject("__case", base);

    return Message(kStrTrue).SetCode(kCodeCase);
  }

  Message When(ObjectMap &p) {
    //TODO:Re-Design
    int size = p.GetVaSize();

    CONDITION_ASSERT(size > 0, "You should provide 1 object at least.");

    ObjectPointer case_head = entry::FindObject("__case");
    string case_content = GetObjectStuff<string>(*case_head);
    string type_id, id;
    bool result = false, state = true;

    size_t count = 0;
    string content;
    for (auto &unit : p) {
      id = "value" + to_string(count);
      if (unit.first == id) {
        if (!IsStringObject(unit.second)) {
          state = false;
          break;
        }

        if (case_content == GetObjectStuff<string>(unit.second)) {
          result = true;
          break;
        }

        count += 1;
      }
    }

    Message msg;

    CONDITION_ASSERT(state, "Case-When is not supported yet.(02)");

    result ? 
      msg = Message(kStrTrue, kCodeWhen, kStrEmpty) : 
      msg = Message(kStrFalse, kCodeWhen, kStrEmpty);
    return msg;
  }

  Message IsNull(ObjectMap &p) {
    auto &obj = p["object"];
    return Message(obj.GetTypeId() == kTypeIdNull ? kStrTrue : kStrFalse);
  }

  void GenericRegister() {
    using namespace entry;
    AddGenericEntry(Entry(Nop, "nop", GT_NOP, kCodeAutoSize));
    AddGenericEntry(Entry(Case, "object", GT_CASE));
    AddGenericEntry(Entry(When, "value", GT_WHEN, kCodeAutoSize));
    AddGenericEntry(Entry(Define, "id|arg", GT_DEF, kCodeAutoSize));
    AddGenericEntry(Entry(ReturnSign, "value", GT_RETURN, kCodeAutoFill));
    AddGenericEntry(Entry(TypeAssert, "object|id", GT_TYPE_ASSERT));
    AddGenericEntry(Entry(TypeAssert, "object|id", GT_ASSERT_R));
    AddGenericEntry(Entry(ArrayMaker, "item", GT_ARRAY, kCodeAutoSize));
    AddGenericEntry(Entry(BindAndSet, "object|source", GT_BIND, kCodeNormalParm, 0));

    AddGenericEntry(Entry(CodeMaker<kCodeTailSign>, "", GT_END));
    AddGenericEntry(Entry(CodeMaker<kCodeConditionLeaf>, "", GT_ELSE));
    AddGenericEntry(Entry(CodeMaker<kCodeContinue>, "", GT_CONTINUE));
    AddGenericEntry(Entry(CodeMaker<kCodeBreak>, "", GT_BREAK));
    AddGenericEntry(Entry(ConditionMaker<kCodeConditionRoot>, "state", GT_IF));
    AddGenericEntry(Entry(ConditionMaker<kCodeHeadSign>, "state", GT_WHILE));
    AddGenericEntry(Entry(ConditionMaker<kCodeConditionBranch>, "state", GT_ELIF));
    AddGenericEntry(BinaryOperator<OperatorCode::ADD, GT_ADD, 2>());
    AddGenericEntry(BinaryOperator<OperatorCode::SUB, GT_SUB, 2>());
    AddGenericEntry(BinaryOperator<OperatorCode::MUL, GT_MUL, 3>());
    AddGenericEntry(BinaryOperator<OperatorCode::DIV, GT_DIV, 3>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::IS, GT_IS, 1>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::LESS_OR_EQUAL, GT_LESS_OR_EQUAL, 1>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::MORE_OR_EQUAL, GT_MORE_OR_EQUAL, 1>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::NOT_EQUAL, GT_NOT_EQUAL, 1>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::MORE, GT_MORE, 1>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::LESS, GT_LESS, 1>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::AND, GT_AND, 1>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::OR, GT_OR, 1>());
    AddGenericEntry(Entry(SelfOperator<false, false>, "object", GT_LSELF_INC));
    AddGenericEntry(Entry(SelfOperator<true, false>, "object", GT_LSELF_DEC));
    AddGenericEntry(Entry(SelfOperator<false, true>, "object", GT_RSELF_INC));
    AddGenericEntry(Entry(SelfOperator<true, true>, "object", GT_RSELF_DEC));
  }

  void BasicUtilityRegister() {
    using namespace entry;
    AddEntry(Entry(WriteLog, kCodeNormalParm, "msg", "log"));
    AddEntry(Entry(Convert, kCodeNormalParm, "object", "convert"));
    AddEntry(Entry(Input, kCodeAutoFill, "msg", "input"));
    AddEntry(Entry(Print, kCodeNormalParm, kStrObject, "print"));
    AddEntry(Entry(GetTimeDate, kCodeNormalParm, "", "time"));
    AddEntry(Entry(Quit, kCodeNormalParm, "", "quit"));
    AddEntry(Entry(GetTypeId, kCodeNormalParm, "object", "typeid"));
    AddEntry(Entry(GetRawStringType, kCodeNormalParm, "object", "type"));
    AddEntry(Entry(Dir, kCodeNormalParm, "object", "dir"));
    AddEntry(Entry(Exist, kCodeNormalParm, "object|id", "exist"));
    AddEntry(Entry(IsNull, kCodeNormalParm, "object", "isnull"));
  }

  void Activiate() {
    using namespace entry;
    GenericRegister();
    BasicUtilityRegister();
    InitPlanners();
#if defined(_WIN32)
    InitLibraryHandler();
#endif
#if not defined(_DISABLE_SDL_)
    LoadSDLStuff();
#endif
  }
}
