#include "machine.h"
#ifndef _NO_CUI_
#include <iostream>
#endif

namespace kagami {
  enum GroupTypeEnum { G_INT, G_DOUBLE, G_STR, G_NUL } ;

  GroupTypeEnum GetGroupType(TokenTypeEnum dataTypeA, TokenTypeEnum dataTypeB,
    string dataA, string dataB) {
    Kit kit;

    GroupTypeEnum groupType = GroupTypeEnum::G_NUL;
    if (dataTypeA == T_DOUBLE || dataTypeB == T_DOUBLE) groupType = G_DOUBLE;
    if (dataTypeA == T_INTEGER && dataTypeB == T_INTEGER) groupType = G_INT;
    if (kit.IsString(dataA) || kit.IsString(dataB)) groupType = G_STR;
    return groupType;
  }

  string BinaryOperations(Object &A, Object &B, string OP) {
    Kit kit;
    string temp;
    using entry::OperatorCode;
    auto OPCode = entry::GetOperatorCode(OP);
    auto dataA = *static_pointer_cast<string>(A.Get()),
      dataB = *static_pointer_cast<string>(B.Get());
    auto dataTypeA = A.GetTokenType(), dataTypeB = B.GetTokenType();
    auto groupType = GetGroupType(dataTypeA, dataTypeB, dataA, dataB);

    if (groupType == G_INT || groupType == G_DOUBLE) {
      switch (OPCode) {
      case OperatorCode::ADD:
      case OperatorCode::SUB:
      case OperatorCode::MUL:
      case OperatorCode::DIV:
        switch (groupType) {
        case G_INT:temp = to_string(kit.Calc(stoi(dataA), stoi(dataB), OP)); break;
        case G_DOUBLE:temp = to_string(kit.Calc(stod(dataA), stod(dataB), OP)); break;
        default:break;
        }
        break;
      case OperatorCode::IS:
      case OperatorCode::MORE_OR_EQUAL:
      case OperatorCode::LESS_OR_EQUAL:
      case OperatorCode::NOT_EQUAL:
      case OperatorCode::MORE:
      case OperatorCode::LESS:
        switch (groupType) {
        case G_INT:
          kit.Logic(stoi(dataA), stoi(dataB), OP) ? temp = kStrTrue : temp = kStrFalse;
          break;
        case G_DOUBLE:
          kit.Logic(stod(dataA), stod(dataB), OP) ? temp = kStrTrue : temp = kStrFalse;
          break;
        }
        break;
      case OperatorCode::BIT_AND:
      case OperatorCode::BIT_OR:

        break;
      default:
        break;
      }
    }
    else if (groupType = G_STR) {
      switch (OPCode) {
      case OperatorCode::ADD:
        if (dataA.front() != '\'') dataA = "'" + dataA;
        if (dataA.back() == '\'') dataA = dataA.substr(0, dataA.size() - 1);
        if (dataB.front() == '\'') dataB = dataB.substr(1, dataB.size() - 1);
        if (dataB.back() != '\'') dataB = dataB + "'";
        temp = dataA + dataB;
        break;
      case OperatorCode::IS:
      case OperatorCode::NOT_EQUAL:
        kit.Logic(dataA, dataB, OP) ? temp = kStrTrue : temp = kStrFalse;
        break;
      case OperatorCode::AND:
        if ((dataA == kStrTrue || dataA == kStrFalse)
          && (dataB == kStrTrue || dataB == kStrFalse)) {
          if (dataA == kStrTrue && dataB == kStrTrue) temp = kStrTrue;
          else temp = kStrFalse;
        }
        else {
          temp = kStrFalse;
        }
        break;
      case OperatorCode::OR:
        if((dataA == kStrTrue || dataA == kStrFalse)
          && (dataB == kStrTrue || dataB == kStrFalse)) {
          if (dataA == kStrTrue || dataB == kStrTrue) temp = kStrTrue;
          else temp = kStrFalse;
        }
        else {
          temp = kStrFalse;
        }
        break;
      default:
        break;
      }
    }
    return temp;
  }

  Message Plus(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "+")); }
  Message Sub(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "-")); }
  Message Multiply(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "*")); }
  Message Divide(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "/")); }
  Message Less(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "<")); }
  Message More(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], ">")); }
  Message LessOrEqual(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "<=")); }
  Message MoreOrEqual(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], ">=")); }
  Message And(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "&&")); }
  Message Or(ObjectMap &p) { return Message(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "||")); }

  Message End(ObjectMap &p) { return Message(kStrEmpty, kCodeTailSign, kStrEmpty); }
  Message Else(ObjectMap &p) { return Message(kStrTrue, kCodeConditionLeaf, kStrEmpty); }
  Message If(ObjectMap &p) { return Message(*static_pointer_cast<string>(p["state"].Get()), kCodeConditionRoot, kStrEmpty); }
  Message Elif(ObjectMap &p) { return Message(*static_pointer_cast<string>(p["state"].Get()), kCodeConditionBranch, kStrEmpty); }
  Message While(ObjectMap &p) { return Message(*static_pointer_cast<string>(p["state"].Get()), kCodeHeadSign, kStrEmpty); }
  Message Continue(ObjectMap &p) { return Message(kStrEmpty, kCodeContinue, kStrEmpty); }
  Message Break(ObjectMap &p) { return Message(kStrEmpty, kCodeBreak, kStrEmpty); }

  //pending modify
  Message LogicEqual(ObjectMap &p) {
    Object objFirst = p["first"], objSecond = p["second"];
    Message msg;
    if (objFirst.GetTypeId() != kTypeIdRawString 
      || objSecond.GetTypeId() != kTypeIdRawString) {
      auto ent = entry::Order("__compare", objFirst.GetTypeId(), 1);
      if (ent.Good()) {
        msg = ent.Start(p);
      }
    }
    else {
      msg.combo(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "=="));
    }
    return msg;
  }
  Message LogicNotEqual(ObjectMap &p) {
    Object objFirst = p["first"], objSecond = p["second"];
    Message msg;
    if (objFirst.GetTypeId() != kTypeIdRawString
      || objSecond.GetTypeId() != kTypeIdRawString) {
      auto ent = entry::Order("__compare", objFirst.GetTypeId(), 1);
      if (ent.Good()) {
        msg = ent.Start(p);
      }
      if (msg.GetDetail() == kStrTrue) msg.SetDetail(kStrFalse);
      else msg.SetDetail(kStrTrue);
    }
    else {
      msg.combo(kStrRedirect, kCodeSuccess, BinaryOperations(p["first"], p["second"], "!="));
    }
    return msg;
  }

  Message Define(ObjectMap &p) {
    vector<string> defHead;
    Object &id = p["id"];
    size_t count = 0;
    defHead.emplace_back(GetObjectStuff<string>(id));

    for (auto &unit : p) {
      if (unit.first == "arg" + to_string(count)) {
        string str = *static_pointer_cast<string>(unit.second.Get());
        defHead.emplace_back(str);
        count++;
      }
    }

    string defHeadStr = Kit::CombineStringVector(defHead);
    return Message(kStrEmpty, kCodeDefineSign, defHeadStr);
  }

  Message ReturnSign(ObjectMap &p) {
    Object &valueObj = p["value"];
    string typeId = valueObj.GetTypeId();
    Object obj;
    if (typeId != kTypeIdNull) {
      obj.Set(valueObj.Get(), typeId)
        .SetMethods(valueObj.GetMethods());
      entry::GetCurrentManager().Add(kStrRetValue, obj);
    }
    return Message(kStrStopSign, kCodeSuccess, kStrEmpty);
  }

  Message WriteLog(ObjectMap &p) {
    Message result;
    auto data = p["msg"];
    ofstream ofs("kagami-script.log", std::ios::out | std::ios::app);

    if (data.GetTypeId() == kTypeIdRawString) {
      const auto ptr = static_pointer_cast<string>(data.Get());
      if (Kit::IsString(*ptr)) {
        ofs << ptr->substr(1, ptr->size() - 2) << "\n";
      }
      else {
        ofs << *ptr << "\n";
      }
    }

    ofs.close();
    return result;
  }

  Message LeftSelfIncreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if(object.GetTypeId() == kTypeIdRawString) {
      const auto origin = GetObjectStuff<string>(object);
      if (object.GetTokenType() == T_INTEGER) {
        auto data = stoi(origin);
        ++data;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
      else if (object.GetTokenType() == T_DOUBLE) {
        auto data = stod(origin);
        data += 1.0f;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message LeftSelfDecreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      const auto origin = GetObjectStuff<string>(object);
      if (object.GetTokenType() == T_INTEGER) {
        auto data = stoi(origin);
        --data;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
      else if (object.GetTokenType() == T_DOUBLE) {
        auto data = stod(origin);
        data -= 1.0f;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message RightSelfIncreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      auto origin = GetObjectStuff<string>(object);
      result = origin;
      if (object.GetTokenType() == T_INTEGER) {
        auto data = stoi(origin);
        ++data;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
      else if (object.GetTokenType() == T_DOUBLE) {
        auto data = stod(origin);
        data += 1.0f;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message RightSelfDecreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      auto origin = GetObjectStuff<string>(object);
      result = origin;
      if (object.GetTokenType() == T_INTEGER) {
        auto data = stoi(origin);
        --data;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
      else if (object.GetTokenType() == T_DOUBLE) {
        auto data = stod(origin);
        data -= 1.0f;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message GetRawStringType(ObjectMap &p) {
    Object &obj = p["object"];
    string result;
    if (obj.GetTypeId() == kTypeIdRawString) {
      string str = GetObjectStuff<string>(obj);
      Kit::IsString(str) ? str = Kit::GetRawString(str) : str = str;
      switch (Kit::GetTokenType(str)) {
      case TokenTypeEnum::T_BOOLEAN:result = "'boolean'"; break;
      case TokenTypeEnum::T_GENERIC:result = "'generic'"; break;
      case TokenTypeEnum::T_INTEGER:result = "'integer'"; break;
      case TokenTypeEnum::T_DOUBLE:result = "'double'"; break;
      case TokenTypeEnum::T_SYMBOL:result = "'symbol'"; break;
      case TokenTypeEnum::T_BLANK:result = "'blank'"; break;
      case TokenTypeEnum::T_STRING:result = "'string'"; break;
      case TokenTypeEnum::T_NUL:result = "'null'"; break;
      default:result = "'null'"; break;
      }
    }
    else {
      result = "'null'";
    }
    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message GetTypeId(ObjectMap &p) {
    Object &obj = p["object"];
    return Message(kStrRedirect, kCodeSuccess, obj.GetTypeId());
  }

  Message BindAndSet(ObjectMap &p) {
    Object &obj = p["object"], source = p["source"];
    Object *targetObj = nullptr;
    bool existed;
    Message msg;
    string objId;
    if (obj.IsRef()) {
      existed = true;
      targetObj = &obj;
    }
    else {
      if (obj.GetTypeId() != kTypeIdRawString) {
        msg.combo(kStrFatalError, kCodeIllegalParm, "Illegal bind operation.");
        return msg;
      }
      objId = GetObjectStuff<string>(obj);
      targetObj = entry::FindObject(objId);
      if (targetObj == nullptr) existed = false;
      else existed = true;
    }
    
    if (existed) {
      if (targetObj->IsRo()) {
        msg.combo(kStrFatalError, kCodeIllegalCall, "Object is read-only.");
      }
      else {
        auto copy = type::GetObjectCopy(source);
        targetObj->Set(copy, source.GetTypeId())
          .SetMethods(source.GetMethods())
          .SetTokenType(source.GetTokenType());
      }
    }
    else {
      auto copy = type::GetObjectCopy(source);
      Object base;
      base.Set(copy, source.GetTypeId())
        .SetMethods(source.GetMethods())
        .SetTokenType(source.GetTokenType())
        .SetRo(false);
      auto result = entry::CreateObject(objId, base);
      if (result == nullptr) {
        msg.combo(kStrFatalError, kCodeIllegalCall, "Object creation failed.");
      }
    }
    return msg;
  }

  Message Print(ObjectMap &p) {
    Message result;
    auto &object = p[kStrObject];
    auto errorMsg = []() {
      std::cout << "You can't print this object." << std::endl;
    };

    if (Kit::FindInStringGroup("__print", object.GetMethods())) {
      auto provider = entry::Order("__print", object.GetTypeId(), -1);
      if (provider.Good()) {
        result = provider.Start(p);
      }
      else {
        errorMsg();
      }
    } 
    else {
      errorMsg();
    }

    return result;
  }

  Message TimeReport(ObjectMap &p) {
    auto now = time(nullptr);
#if defined(_WIN32) && defined(_MSC_VER)
    char nowTime[30] = { ' ' };
    ctime_s(nowTime, sizeof(nowTime), &now);
    string str(nowTime);
    str.pop_back(); //erase '\n'
    return Message(kStrRedirect, kCodeSuccess, "'" + str + "'");
#else
    string TimeData(ctime(&now));
    return Message(kStrRedirect, kCodeSuccess, "'" + TimeData + "'");
#endif
  }

  Message Input(ObjectMap &p) {
    auto it = p.find("msg");
    if (it != p.end()) {
      ObjectMap objMap;
      objMap.insert(ObjectPair("not_wrap", Object()));
      objMap.insert(ObjectPair(kStrObject, it->second));
      //objMap.insert(ObjectPair("object", it->second));
      Print(objMap);
    }
    string buf;
    std::getline(std::cin, buf);
    return Message(kStrRedirect, kCodeSuccess, "'" + buf + "'");
  }

  Message Convert(ObjectMap &p) {
    auto &str = p["object"];
    if (str.GetTypeId() != kTypeIdRawString) {
      return Message(kStrFatalError, kCodeBadExpression, "Cannot convert to basic type(01)");
    }
    Object obj;
    string origin = *static_pointer_cast<string>(str.Get());
    if (Kit::IsString(origin)) origin = Kit::GetRawString(origin);
    auto type = Kit::GetTokenType(origin);
    if (type == T_NUL || type == T_GENERIC) {
      return Message(kStrFatalError, kCodeBadExpression, "Cannot convert to basic type(02)");
    }
    obj.Manage(origin)
      .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
      .SetTokenType(type);
    Message msg;
    msg.SetObject(obj, "__result");
    return msg;
  }

  Message Quit(ObjectMap &p) {
    Message result(kStrEmpty, kCodeQuit, kStrEmpty);
    return result;
  }

  Message Nop(ObjectMap &p) {
    Object &objSize = p["__size"];
    int size = stoi(GetObjectStuff<string>(objSize));
    Object &lastObj = p["nop" + to_string(size - 1)];
    Message msg;
    msg.SetObject(lastObj, "__result");
    return msg;
  }

  Message ArrayMaker(ObjectMap &p) {
    Object &objSize = p["__size"];
    int size = stoi(GetObjectStuff<string>(objSize));
    vector<Object> base;
    Message msg;
    base.reserve(size);
    if (!p.empty()) {
      for (int i = 0; i < size; i++) {
        base.emplace_back(p["item" + to_string(i)]);
      }
    }
    msg.SetObject(Object()
      .SetConstructorFlag()
      .Set(make_shared<vector<Object>>(base), kTypeIdArrayBase)
      .SetMethods(type::GetPlanner(kTypeIdArrayBase)->GetMethods())
      .SetRo(false)
      , "__result");
    return msg;
  }

  Message Dir(ObjectMap &p) {
    Object &obj = p["object"];
    auto vec = Kit::BuildStringVector(obj.GetMethods());
    Message msg;
    vector<Object> output;
    for (auto &unit : vec) {
      output.emplace_back(Object()
        .Set(make_shared<string>(unit),kTypeIdString)
        .SetMethods(type::GetPlanner(kTypeIdString)->GetMethods())
        .SetRo(true)
      );
      msg.SetObject(Object()
        .SetConstructorFlag()
        .Set(make_shared<vector<Object>>(output), kTypeIdArrayBase)
        .SetMethods(type::GetPlanner(kTypeIdArrayBase)->GetMethods())
        .SetRo(true), "__result");
    }
    return msg;
  }


  Message Exist(ObjectMap &p){
    Object &obj = p["object"];
    string target = Kit::GetRawString(GetObjectStuff<string>(p["id"]));
    bool result = Kit::FindInStringGroup(target, obj.GetMethods());
    Message msg;
    result ? 
      msg = Message(kStrRedirect, kCodeSuccess, kStrTrue): 
      msg = Message(kStrRedirect, kCodeSuccess, kStrFalse);
    return msg;
  }

  Message TypeAssert(ObjectMap &p) {
    Object &obj = p["object"];
    string target = GetObjectStuff<string>(p["id"]);
    bool result = Kit::FindInStringGroup(target, obj.GetMethods());
    Message msg;
    result ?
      msg = Message(kStrRedirect, kCodeSuccess, kStrTrue) :
      msg = Message(kStrFatalError,kCodeIllegalCall,"Method not found - " + target);
    return msg;
  }

  Message Case(ObjectMap &p) {
    Object &obj = p["object"];
    auto typeId = obj.GetTypeId();
    if (typeId != kTypeIdRawString || typeId != kTypeIdString) {
      //TODO:Re-design
      return Message(kStrFatalError, kCodeIllegalParm, "Case-When is not supported yet.(01)");
    }
    auto copy = type::GetObjectCopy(obj);
    Object base;
    base.Set(copy, obj.GetTypeId())
      .SetMethods(obj.GetMethods())
      .SetRo(false);
    entry::CreateObject("__case", base);
    return Message(kStrRedirect, kCodeCase, kStrTrue);
  }

  Message When(ObjectMap &p) {
    //TODO:Re-Design
    Object &objSize = p["__size"];
    int size = stoi(GetObjectStuff<string>(objSize));
    Object *caseHead = entry::FindObject("__case");
    string caseContent = GetObjectStuff<string>(*caseHead);
    string typeId;
    bool result = false, state = true;
    for (int i = 0; i < size; ++i) {
      Object &obj = p["case" + to_string(i)];
      typeId = obj.GetTypeId();
      if (typeId != kTypeIdRawString || typeId != kTypeIdString) {
        state = false;
        break;
      }
      string content = GetObjectStuff<string>(obj);
      if (content == caseContent) {
        result = true;
        break;
      }
    }

    Message msg;
    if (!state) {
      msg.combo(kStrFatalError, kCodeIllegalParm, "Case-When is not supported yet.(02)");
    }
    result ? 
      msg = Message(kStrRedirect, kCodeSuccess, kStrTrue) : 
      msg = Message(kStrRedirect, kCodeSuccess, kStrFalse);
    return msg;
  }

  void GenericRegister() {
    using namespace entry;
    AddGenericEntry(GT_NOP, Entry(Nop, "nop", GT_NOP, kCodeAutoSize));
    AddGenericEntry(GT_ARRAY, Entry(ArrayMaker, "item", GT_ARRAY, kCodeAutoSize));
    AddGenericEntry(GT_END, Entry(End, "", GT_END));
    AddGenericEntry(GT_ELSE, Entry(Else, "", GT_ELSE));
    AddGenericEntry(GT_IF, Entry(If, "state", GT_IF));
    AddGenericEntry(GT_WHILE, Entry(While, "state", GT_WHILE));
    AddGenericEntry(GT_ELIF, Entry(Elif, "state", GT_ELIF));
    AddGenericEntry(GT_BIND, Entry(BindAndSet, "object|source", GT_BIND, kCodeNormalParm, 0));
    AddGenericEntry(GT_ADD, Entry(Plus, "first|second", GT_ADD, kCodeNormalParm, 2));
    AddGenericEntry(GT_SUB, Entry(Sub, "first|second", GT_SUB, kCodeNormalParm, 2));
    AddGenericEntry(GT_MUL, Entry(Multiply, "first|second", GT_MUL, kCodeNormalParm, 3));
    AddGenericEntry(GT_DIV, Entry(Divide, "first|second", GT_DIV, kCodeNormalParm, 3));
    AddGenericEntry(GT_IS, Entry(LogicEqual, "first|second", GT_IS, kCodeNormalParm, 1));
    AddGenericEntry(GT_LESS_OR_EQUAL, Entry(LessOrEqual, "first|second", GT_LESS_OR_EQUAL, kCodeNormalParm, 1));
    AddGenericEntry(GT_MORE_OR_EQUAL, Entry(MoreOrEqual, "first|second", GT_MORE_OR_EQUAL, kCodeNormalParm, 1));
    AddGenericEntry(GT_NOT_EQUAL, Entry(LogicNotEqual, "first|second", GT_NOT_EQUAL, kCodeNormalParm, 1));
    AddGenericEntry(GT_MORE, Entry(More, "first|second", GT_MORE, kCodeNormalParm, 1));
    AddGenericEntry(GT_LESS, Entry(Less, "first|second", GT_LESS, kCodeNormalParm, 1));
    AddGenericEntry(GT_LSELF_INC, Entry(LeftSelfIncreament, "object", GT_LSELF_INC));
    AddGenericEntry(GT_LSELF_DEC, Entry(LeftSelfDecreament, "object", GT_LSELF_DEC));
    AddGenericEntry(GT_RSELF_INC, Entry(RightSelfIncreament, "object", GT_RSELF_INC));
    AddGenericEntry(GT_RSELF_DEC, Entry(RightSelfDecreament, "object", GT_RSELF_DEC));
    AddGenericEntry(GT_AND, Entry(And, "first|second", GT_AND, kCodeNormalParm, 1));
    AddGenericEntry(GT_OR, Entry(Or, "first|second", GT_OR, kCodeNormalParm, 1));
    AddGenericEntry(GT_DEF, Entry(Define, "id|arg", GT_DEF, kCodeAutoSize));
    AddGenericEntry(GT_RETURN, Entry(ReturnSign, "value", GT_RETURN, kCodeAutoFill));
    AddGenericEntry(GT_TYPE_ASSERT, Entry(TypeAssert, "object|id", GT_TYPE_ASSERT));
    AddGenericEntry(GT_CONTINUE, Entry(Continue, "", GT_CONTINUE));
    AddGenericEntry(GT_BREAK, Entry(Break, "", GT_BREAK));
    AddGenericEntry(GT_CASE, Entry(Case, "object", GT_CASE));
    AddGenericEntry(GT_WHEN, Entry(When, "case", GT_WHEN));
  }

  void BasicUtilityRegister() {
    using namespace entry;
    AddEntry(Entry(WriteLog, kCodeNormalParm, "msg", "log"));
    AddEntry(Entry(Convert, kCodeNormalParm, "object", "convert"));
    AddEntry(Entry(Input, kCodeAutoFill, "msg", "input"));
    AddEntry(Entry(Print, kCodeNormalParm, kStrObject, "print"));
    AddEntry(Entry(TimeReport, kCodeNormalParm, "", "time"));
    AddEntry(Entry(Quit, kCodeNormalParm, "", "quit"));
    AddEntry(Entry(GetTypeId, kCodeNormalParm, "object", "typeid"));
    AddEntry(Entry(GetRawStringType, kCodeNormalParm, "object", "type"));
    AddEntry(Entry(Dir, kCodeNormalParm, "object", "dir"));
    AddEntry(Entry(Exist, kCodeNormalParm, "object|id", "exist"));
  }

  void Activiate() {
    using namespace entry;
    GenericRegister();
    BasicUtilityRegister();
    InitPlanners();
#if defined(_ENABLE_DEBUGGING_)
    LoadSDLStuff();
#endif
  }
}
