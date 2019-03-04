#include "component.h"

namespace kagami {
  Message Print(ObjectMap &p) {
    Object &obj = p[kStrObject];
    string type_id = obj.GetTypeId();
    if (util::IsPlainType(type_id)) {
      if (type_id == kTypeIdInt) {
        std::cout << obj.Cast<long>() << std::flush;
      }
      else if (type_id == kTypeIdFloat) {
        std::cout << obj.Cast<double>() << std::flush;
      }
      else if (type_id == kTypeIdString) {
        std::cout << obj.Cast<string>() << std::flush;
      }
      else if (type_id == kTypeIdBool) {
        std::cout << obj.Cast<bool>() << std::flush;
      }

      return Message();
    }

    vector<string> methods = management::type::GetMethods(obj.GetTypeId());

    auto errorMsg = []() {
      std::cout << "You can't print this object." << std::endl;
    };

    if (!find_in_vector<string>(kStrPrint, methods)) {
      errorMsg();
      return Message();
    }

    return management::FindInterface(kStrPrint, obj.GetTypeId()).Start(p);
  }

  Message PrintLine(ObjectMap &p) {
    Message msg = Print(p);
    if (msg.GetLevel() == kStateNormal) {
      std::cout << std::endl;
    }

    return msg;
  }

  Message Input(ObjectMap &p) {
    if (!p["msg"].Null()) {
      EXPECT(IsStringFamily(p["msg"]),
        "Illegal message string.");
      
      ObjectMap obj_map = {
        NamedObject(kStrObject, p["msg"])
      };

      Print(obj_map);
    }

    string buf;
    std::getline(std::cin, buf);
    DEBUG_EVENT("(Input Interface)Content:" + buf);
    return Message().SetObject(buf);
  }


  //TODO:Rewrite
  Message Convert(ObjectMap &p) {
    EXPECT_TYPE(p, "object", kTypeIdString);

    string origin = p.Cast<string>("object");
    auto type = util::GetTokenType(origin);
    Message msg;
    string str;

    switch (type) {
    case kTokenTypeInt:
      msg.SetObject(stol(origin));
      break;
    case kTokenTypeFloat:
      msg.SetObject(stod(origin));
      break;
    case kTokenTypeBool:
      msg.SetObject(origin == kStrTrue);
    default:
      msg.SetObject(origin);
      break;
    }
    
    return msg;
  }

  Message IsNull(ObjectMap &p) {
    auto &obj = p["object"];
    return Message(obj.GetTypeId() == kTypeIdNull ? kStrTrue : kStrFalse);
  }

  Message Time(ObjectMap &p) {
    auto now = time(nullptr);
    string nowtime(ctime(&now));
    nowtime.pop_back();

    return Message(nowtime);
  }

  Message UseCount(ObjectMap &p) {
    long count = p["object"].use_count();
    return Message().SetObject(count);
  }

  Message Destroy(ObjectMap &p) {
    Object &obj = p["object"].Deref();

    obj.swap(Object());

    return Message();
  }

  Message Version(ObjectMap &p) {
    return Message().SetObject(kInterpreterVersion);
  }

  Message PatchVersion(ObjectMap &p) {
    return Message().SetObject(kPatchName);
  }

  void Activiate() {
    using management::CreateNewInterface;

    CreateNewInterface(Interface(Convert, "object", "convert"));
    CreateNewInterface(Interface(Input, "msg", "input", kCodeAutoFill));
    CreateNewInterface(Interface(Print, kStrObject, "print"));
    CreateNewInterface(Interface(PrintLine, kStrObject, "println"));
    CreateNewInterface(Interface(IsNull, "object", "null"));
    CreateNewInterface(Interface(Time, "", "time"));
    CreateNewInterface(Interface(UseCount, "object", "use_count"));
    CreateNewInterface(Interface(Destroy, "object", "destroy"));
    CreateNewInterface(Interface(DecimalConvert<2>, "str", "bin"));
    CreateNewInterface(Interface(DecimalConvert<8>, "str", "octa"));
    CreateNewInterface(Interface(DecimalConvert<16>, "str", "hex"));
    CreateNewInterface(Interface(Version, "", "_version"));
    CreateNewInterface(Interface(PatchVersion, "", "_patch"));

    auto create_constant = [](string id, string content) {
      management::CreateConstantObject(
        id, Object(content)
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
