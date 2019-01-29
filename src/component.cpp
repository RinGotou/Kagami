#include "component.h"

namespace kagami {
  PairTypePolicy GetTypePolicy(Object &A, Object &B) {
    PairTypePolicy policy = PairTypePolicy::G_OTHER_OBJ;

    if (A.GetTypeId() == kTypeIdRawString && B.GetTypeId() == kTypeIdRawString) {
      string data_A = A.Cast<string>();
      string data_B = B.Cast<string>();
      TokenType type_A = util::GetTokenType(data_A);
      TokenType type_B = util::GetTokenType(data_B);

      if (type_A == kTokenTypeFloat || type_B == kTokenTypeFloat) policy = G_FLOAT;
      if (type_A == kTokenTypeInt && type_B == kTokenTypeInt) policy = G_INT;
      if (util::IsString(data_A) || util::IsString(data_B)) policy = G_STR;
      if (util::IsBoolean(data_A) && util::IsBoolean(data_B)) policy = G_STR;
    }

    return policy;
  }

  Message GetRawStringType(ObjectMap &p) {
    OBJECT_ASSERT(p, "object", kTypeIdRawString);

    string result;
    string str = RealString(p.Cast<string>("object"));

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
    vector<string> methods = management::type::GetMethods(obj.GetTypeId());

    auto errorMsg = []() {
      std::cout << "You can't print this object." << std::endl;
    };

    if (!find_in_vector<string>(kStrPrint, methods)) {
      errorMsg();
      return Message();
    }

    return management::Order(kStrPrint, obj.GetTypeId()).Start(p);
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
      
      ObjectMap obj_map = {
        NamedObject("not_wrap", Object(kStrTrue)),
        NamedObject(kStrObject, p["msg"])
      };

      Print(obj_map);
    }

    string buf;
    std::getline(std::cin, buf);
    return Message("'" + buf + "'");
  }

  Message Convert(ObjectMap &p) {
    OBJECT_ASSERT(p, "object", kTypeIdRawString);

    string origin = RealString(p.Cast<string>("object"));
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

  void Activiate() {
    using management::CreateInterface;

    OperatorGenerator<PLUS, kTokenPlus>();
    OperatorGenerator<MINUS, kTokenMinus>();
    OperatorGenerator<TIMES, kTokenTimes>();
    OperatorGenerator<DIV, kTokenDivide>();
    OperatorGenerator<EQUALS, kTokenEquals, true>();
    OperatorGenerator<NOT_EQUAL, kTokenNotEqual, true>();
    OperatorGenerator<LESS_OR_EQUAL, kTokenLessOrEqual, true>();
    OperatorGenerator<GREATER_OR_EQUAL, kTokenGreaterOrEqual, true>();
    OperatorGenerator<GREATER, kTokenGreater, true>();
    OperatorGenerator<LESS, kTokenLess, true>();
    OperatorGenerator<AND, kTokenAnd, true>();
    OperatorGenerator<OR, kTokenOr, true>();

    CreateInterface(Interface(Convert, "object", "convert"));
    CreateInterface(Interface(Input, "msg", "input", kCodeAutoFill));
    CreateInterface(Interface(Print, kStrObject, "print"));
    CreateInterface(Interface(GetTimeDate, "", "time"));
    CreateInterface(Interface(GetRawStringType, "object", "type"));
    CreateInterface(Interface(IsNull, "object", "isnull"));

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
