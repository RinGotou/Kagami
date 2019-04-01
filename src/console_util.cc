#include "machine_kisaragi.h"

namespace kagami {
  Message SystemCommand(ObjectMap &p) {
    EXPECT_TYPE(p, "command", kTypeIdString);
    system(p.Cast<string>("command").c_str());
    return Message();
  }

  //Print single object
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

      CHECK_PRINT_OPT();

      return Message();
    }

    vector<string> methods = management::type::GetMethods(obj.GetTypeId());

    if (!find_in_vector<string>(kStrPrint, methods)) {
      std::cout << "You can't print this object." << std::endl;
      return Message();
    }

    return MakeInvokePoint(kStrPrint, obj.GetTypeId());
  }

  //Print object and switch to next line
  Message PrintLine(ObjectMap &p) {
    p.insert(NamedObject(kStrSwitchLine, Object()));
    Message msg = Print(p);
    return msg;
  }

  Message Input(ObjectMap &p) {
    if (!p["msg"].Null()) {
      EXPECT(IsStringFamily(p["msg"]), "Illegal message string.");
      
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

  Message GetChar(ObjectMap &p) {
    string str;
    str.append(1, static_cast<char>(getchar()));
    return Message().SetObject(str);
  }

  void InitConsoleComponents() {
    using management::CreateNewInterface;

    CreateNewInterface(Interface(Input, "msg", "input", kCodeAutoFill));
    CreateNewInterface(Interface(GetChar, "", "getchar"));
    CreateNewInterface(Interface(Print, kStrObject, "print"));
    CreateNewInterface(Interface(PrintLine, kStrObject, "println"));
    CreateNewInterface(Interface(SystemCommand, "command", "console"));
  }
}
