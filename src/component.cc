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

  Message IsNull(ObjectMap &p) {
    auto &obj = p["object"];
    return Message().SetObject(obj.GetTypeId() == kTypeIdNull);
  }

  Message Time(ObjectMap &p) {
    auto now = time(nullptr);
    string nowtime(ctime(&now));
    nowtime.pop_back();

    return Message(nowtime);
  }

  Message Version(ObjectMap &p) {
    return Message().SetObject(kInterpreterVersion);
  }

  Message PatchVersion(ObjectMap &p) {
    return Message().SetObject(kPatchName);
  }

  void Activiate() {
    using management::CreateNewInterface;

    CreateNewInterface(Interface(Input, "msg", "input", kCodeAutoFill));
    CreateNewInterface(Interface(Print, kStrObject, "print"));
    CreateNewInterface(Interface(PrintLine, kStrObject, "println"));
    CreateNewInterface(Interface(Time, "", "time"));
    CreateNewInterface(Interface(DecimalConvert<2>, "str", "bin"));
    CreateNewInterface(Interface(DecimalConvert<8>, "str", "octa"));
    CreateNewInterface(Interface(DecimalConvert<16>, "str", "hex"));
    CreateNewInterface(Interface(Version, "", "_version"));
    CreateNewInterface(Interface(PatchVersion, "", "_patch"));

    EXPORT_CONSTANT(kInterpreterVersion);
    EXPORT_CONSTANT(kPlatformType);
    EXPORT_CONSTANT(kPatchName);
  }
}
