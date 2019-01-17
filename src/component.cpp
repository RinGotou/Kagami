#include "component.h"

namespace kagami {
  template <class T>
  Object MakeObject(T t) {
    string str = to_string(t);
    return Object(str, util::GetTokenType(str));
  }

  PairTypePolicy GetTypePolicy(Object &A, Object &B) {
    auto data_A = GetObjectStuff<string>(A),
      data_B = GetObjectStuff<string>(B);
    auto data_type_A = util::GetTokenType(data_A);
    auto data_type_B = util::GetTokenType(data_B);

    PairTypePolicy policy = PairTypePolicy::G_NUL;

    if (data_type_A == T_FLOAT || data_type_B == T_FLOAT) {
      policy = G_FLOAT;
    }
    if (data_type_A == T_INTEGER && data_type_B == T_INTEGER) {
      policy = G_INT;
    }
    if (util::IsString(data_A) || util::IsString(data_B)) {
      policy = G_STR;
    }
    if ((data_A == kStrTrue || data_A == kStrFalse) &&
      (data_B == kStrTrue || data_B == kStrFalse)) {
      policy = G_STR;
    }

    return policy;
  }

  inline bool CheckObjectType(Object &obj, string type_id) {
    return (obj.GetTypeId() == type_id);
  }

  inline bool IsRawStringObject(Object &obj) {
    return CheckObjectType(obj, kTypeIdRawString);
  }

  inline Message IllegalCallMsg(string str) {
    return Message(kCodeIllegalCall, str, kStateError);
  }

  inline Message IllegalParamMsg(string str) {
    return Message(kCodeIllegalParam, str, kStateError);
  }

  inline Message CheckEntryAndStart(string id, string type_id, ObjectMap &param) {
    Message msg;
    auto entry = management::Order(id, type_id);
    entry.Good() ?
      msg = entry.Start(param) :
      msg.SetCode(kCodeIllegalCall);
    return msg;
  }

  inline void CheckSelfOperatorMsg(Message &msg, string res) {
    res.empty() ?
      msg = Message(res) :
      msg = IllegalParamMsg("Illegal self-operator.");
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
      if (tempMsg.GetCode() == kCodeIllegalCall) {
        errorMsg();
      }
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
      str = "" :
      str = origin;
    
    return Message().SetObject(Object(str));
  }

  Message IsNull(ObjectMap &p) {
    auto &obj = p["object"];
    return Message(obj.GetTypeId() == kTypeIdNull ? kStrTrue : kStrFalse);
  }

  void OperatorRegister() {
    using namespace management;
    AddGenericEntry(BinaryOperator<OperatorCode::ADD, GT_ADD>());
    AddGenericEntry(BinaryOperator<OperatorCode::SUB, GT_SUB>());
    AddGenericEntry(BinaryOperator<OperatorCode::MUL, GT_MUL>());
    AddGenericEntry(BinaryOperator<OperatorCode::DIV, GT_DIV>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::IS, GT_IS>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::LESS_OR_EQUAL, GT_LESS_OR_EQUAL>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::MORE_OR_EQUAL, GT_MORE_OR_EQUAL>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::NOT_EQUAL, GT_NOT_EQUAL>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::MORE, GT_MORE>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::LESS, GT_LESS>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::AND, GT_AND>());
    AddGenericEntry(LogicBinaryOperator<OperatorCode::OR, GT_OR>());
  }

  void BasicUtilityRegister() {
    using namespace management;
    AddEntry(Entry(Convert, "object", "convert"));
    AddEntry(Entry(Input, "msg", "input", kCodeAutoFill));
    AddEntry(Entry(Print, kStrObject, "print"));
    AddEntry(Entry(GetTimeDate, "", "time"));
    AddEntry(Entry(GetRawStringType, "object", "type"));
    AddEntry(Entry(IsNull, "object", "isnull"));
  }

  void Activiate() {
    OperatorRegister();
    BasicUtilityRegister();
    InitPlanners();

#if not defined(_DISABLE_SDL_)
    LoadSDLStuff();
#endif
  }
}
