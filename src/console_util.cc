#include "machine.h"

namespace kagami {
  inline string MakeObjectString(Object& obj) {
    return string("<Object Type=") + obj.GetTypeId() + string(">");
  }

  Message SystemCommand(ObjectMap &p) {
    EXPECT_TYPE(p, "command", kTypeIdString);
    int64_t result = system(p.Cast<string>("command").data());
    return Message().SetObject(Object(result, kTypeIdInt));
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
    Object &obj = p[kStrMe];
    string type_id = obj.GetTypeId();
    if (util::IsPlainType(type_id)) {
      if (type_id == kTypeIdInt) {
#if not defined (_MSC_VER)
        fprintf(VM_STDOUT, "%ld", obj.Cast<int64_t>());
#else
        fprintf(VM_STDOUT, "%lld", obj.Cast<int64_t>());
#endif
      }
      else if (type_id == kTypeIdFloat) {
        fprintf(VM_STDOUT, "%f", obj.Cast<double>());
      }
      else if (type_id == kTypeIdString) {
        fputs(obj.Cast<string>().data(), VM_STDOUT);
      }
      else if (type_id == kTypeIdBool) {
        fputs(obj.Cast<bool>() ? "true" : "false", VM_STDOUT);
      }

      CHECK_PRINT_OPT();

      return Message();
    }

    vector<string> methods = management::type::GetMethods(obj.GetTypeId());

    if (!management::type::CheckMethod(kStrPrint, obj.GetTypeId())) {
      puts(MakeObjectString(obj).data());
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
    auto &msg = p["msg"];
    auto type_id = msg.GetTypeId();

    if (!msg.Null()) {
      EXPECT(type_id == kTypeIdString || type_id == kTypeIdWideString,
        "Illegal message string.");
      
      ObjectMap obj_map = {
        NamedObject(kStrMe, p["msg"])
      };

      Print(obj_map);
    }

    string buf = GetLine();
    DEBUG_EVENT("(Input Interface)Content:" + buf);
    return Message().SetObject(buf);
  }

  Message GetChar(ObjectMap &p) {
    string str;
    str.append(1, static_cast<char>(getchar()));
    return Message().SetObject(str);
  }

  Message Test(ObjectMap &p) {
    return Message();
  }

  void InitConsoleComponents() {
    using management::CreateNewInterface;

    CreateNewInterface(Interface(Input, "msg", "input", kCodeAutoFill));
    CreateNewInterface(Interface(GetChar, "", "getchar"));
    CreateNewInterface(Interface(Print, kStrMe, "print"));
    CreateNewInterface(Interface(PrintLine, kStrMe, "println"));
    CreateNewInterface(Interface(SystemCommand, "command", "console"));
    CreateNewInterface(Interface(ThreadSleep, "milliseconds", "sleep"));
    CreateNewInterface(Interface(Test, "obj", "InvokeTest"));
  }
}
