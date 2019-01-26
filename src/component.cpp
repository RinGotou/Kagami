#include "component.h"

namespace kagami {
  template <class T>
  Object MakeObject(T t) {
    string str = to_string(t);
    return Object(str, util::GetTokenType(str));
  }

  PairTypePolicy GetTypePolicy(Object &A, Object &B) {
    PairTypePolicy policy = PairTypePolicy::G_OTHER_OBJ;

    if (A.GetTypeId() == kTypeIdRawString && B.GetTypeId() == kTypeIdRawString) {
      string data_A = GetObjectStuff<string>(A);
      string data_B = GetObjectStuff<string>(B);
      TokenType type_A = util::GetTokenType(data_A);
      TokenType type_B = util::GetTokenType(data_B);

      if (type_A == kTokenTypeFloat || type_B == kTokenTypeFloat) policy = G_FLOAT;
      if (type_A == kTokenTypeInt && type_B == kTokenTypeInt) policy = G_INT;
      if (util::IsString(data_A) || util::IsString(data_B)) policy = G_STR;
      if (util::IsBoolean(data_A) && util::IsBoolean(data_B)) policy = G_STR;
    }

    return policy;
  }

  inline Message CheckEntryAndStart(string id, string type_id, ObjectMap &param) {
    Message msg;
    auto interface = management::Order(id, type_id);
    interface.Good() ?
      msg = interface.Start(param) :
      msg.SetCode(kCodeIllegalCall);
    return msg;
  }

  Message GetRawStringType(ObjectMap &p) {
    OBJECT_ASSERT(p, "object", kTypeIdRawString);

    string result;
    string str = RealString(p.Get<string>("object"));

    switch (util::GetTokenType(str)) {
    case kTokenTypeBool:result = "'boolean'"; break;
    case kTokenTypeGeneric:result = "'generic'"; break;
    case kTokenTypeInt:result = "'integer'"; break;
    case kTokenTypeFloat:result = "'float'"; break;
    case kTokenTypeSymbol:result = "'symbol'"; break;
    case kTokenTypeBlank:result = "'blank'"; break;
    case kTokenTypeString:result = "'string'"; break;
    case kTokenTypeNull:result = "'null'"; break;
    default:result = "'null'"; break;
    }

    return Message(result);
  }

  Message Print(ObjectMap &p) {
    Object &obj = p[kStrObject];
    string methods = management::type::GetMethods(obj.GetTypeId());

    auto errorMsg = []() {
      std::cout << "You can't print this object." << std::endl;
    };

    if (!util::FindInStringGroup("__print", methods)) {
      errorMsg();
      return Message();
    } 

    Message tempMsg = CheckEntryAndStart("__print", obj.GetTypeId(), p);
    if (tempMsg.GetCode() == kCodeIllegalCall) {
      errorMsg();
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

    compare(type, { kTokenTypeNull,kTokenTypeGeneric }) ?
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
    CreateGenericInterface(OperatorGenerator<PLUS, kTokenPlus>());
    CreateGenericInterface(OperatorGenerator<MINUS, kTokenMinus>());
    CreateGenericInterface(OperatorGenerator<TIMES, kTokenTimes>());
    CreateGenericInterface(OperatorGenerator<DIV, kTokenDivide>());
    CreateGenericInterface(OperatorGenerator<EQUALS, kTokenEquals, true>());
    CreateGenericInterface(OperatorGenerator<NOT_EQUAL, kTokenNotEqual, true>());
    CreateGenericInterface(OperatorGenerator<LESS_OR_EQUAL, kTokenLessOrEqual, true>());
    CreateGenericInterface(OperatorGenerator<GREATER_OR_EQUAL, kTokenGreaterOrEqual, true>());
    CreateGenericInterface(OperatorGenerator<GREATER, kTokenGreater, true>());
    CreateGenericInterface(OperatorGenerator<LESS, kTokenLess, true>());
    CreateGenericInterface(OperatorGenerator<AND, kTokenAnd, true>());
    CreateGenericInterface(OperatorGenerator<OR, kTokenOr, true>());
  }

  void BasicUtilityRegister() {
    using namespace management;

    CreateInterface({
      Interface(Convert, "object", "convert"),
      Interface(Input, "msg", "input", kCodeAutoFill),
      Interface(GetTimeDate, "", "time"),
      Interface(GetRawStringType, "object", "type"),
      Interface(IsNull, "object", "null")
      });
  }

  void Activiate() {
    OperatorRegister();
    BasicUtilityRegister();
    InitPlanners();

#if not defined(_DISABLE_SDL_)
    LoadSDLStuff();
#endif

    auto create_constant = [](string id, string content) {
      management::CreateConstantObject(
        id,
        Object("'" + content + "'")
      );
    };

    create_constant("kVersion", kInterpreterVersion);
    create_constant("kPlatform", kPlatformType);
    create_constant("kInternalName", kPatchName);
    create_constant("kStringTypeBool", "boolean");
    create_constant("kStringTypeGenericId", "generic");
    create_constant("kStringTypeInteger", "integer");
    create_constant("kStringTypeFloat", "float");
    create_constant("kStringTypeSymbol", "symbol");
    create_constant("kStringTypeBlank", "blank");
    create_constant("kStringTypeStr", "string");
    create_constant("kStringTypeNull", "null");
  }
}
