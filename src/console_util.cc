#include "machine.h"

namespace kagami {
  inline string MakeObjectString(Object& obj) {
    return string("<Object Type=") + obj.GetTypeId() + string(">");
  }

  Message SystemCommand(ObjectMap &p) {
    EXPECT_TYPE(p, "command", kTypeIdString);
    system(p.Cast<string>("command").c_str());
    return Message();
  }

  Message ThreadSleep(ObjectMap& p) {
    EXPECT_TYPE(p, "milliseconds", kTypeIdInt);
    auto value = p.Cast<int64_t>("milliseconds");
#if defined (_WIN32)
    Sleep(DWORD(p.Cast<int64_t>("milliseconds")));
#else
    timespec spec;
    
    if (value >= 1000) {
      spec.tv_sec = value / 1000;
      spec.tv_nsec = (value - (static_cast<int64_t>(spec.tv_sec) * 1000))
        * 1000000;
    }
    else {
      spec.tv_sec = 0;
      spec.tv_nsec = value * 1000000;
    }
    
    nanosleep(&spec, nullptr);
#endif

    return Message();
  }

  //Print single object
  Message Print(ObjectMap &p) {
    Object &obj = p[kStrObject];
    string type_id = obj.GetTypeId();
    if (util::IsPlainType(type_id)) {
      if (type_id == kTypeIdInt) {
        std::cout << obj.Cast<int64_t>() << std::flush;
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

    if (!management::type::CheckMethod(kStrPrint, obj.GetTypeId())) {
      std::cout << MakeObjectString(obj) << std::endl;
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
    CreateNewInterface(Interface(ThreadSleep, "milliseconds", "sleep"));
  }
}
