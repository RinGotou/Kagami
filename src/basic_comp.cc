#include "machine.h"

namespace kagami {
  inline string MakeObjectString(Object& obj) {
    return string("<Object Type=") + obj.GetTypeId() + string(">");
  }

  Message SystemCommand(ObjectMap &p) {
    auto tc_result = TypeChecking(
      { Expect("command", kTypeIdString) }, p);

    if (TC_FAIL(tc_result)) { return TC_ERROR(tc_result); }

    int64_t result = system(p.Cast<string>("command").data());
    return Message().SetObject(Object(result, kTypeIdInt));
  }

  Message ThreadSleep(ObjectMap& p) {
    auto tc_result = TypeChecking(
      { Expect("milliseconds", kTypeIdInt) }, p);

    if (TC_FAIL(tc_result)) { return TC_ERROR(tc_result); }

    auto value = p.Cast<int64_t>("milliseconds");
#ifdef _MSC_VER
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
    if (lexical::IsPlainType(type_id)) {
      if (type_id == kTypeIdInt) {
#ifndef _MSC_VER
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

      CHECK_PRINT_OPT(p);

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
      if (type_id != kTypeIdString && type_id != kTypeIdWideString) {
        return Message("Invalid message string", kStateError);
      }
      
      ObjectMap obj_map = {
        NamedObject(kStrMe, p["msg"])
      };

      Print(obj_map);
    }

    string buf = GetLine();
    DEBUG_EVENT("(Input FunctionImpl)Content:" + buf);
    return Message().SetObject(buf);
  }

  Message GetChar(ObjectMap &p) {
    auto value = static_cast<char>(fgetc(VM_STDIN));
    return Message().SetObject(string().append(1, value));
  }

  Message SetWorkingDir(ObjectMap &p) {
    auto tc_result = TypeChecking(
      { Expect("dir", kTypeIdString) }, p);

    if (TC_FAIL(tc_result)) return TC_ERROR(tc_result);

    auto &dir = p.Cast<string>("dir");
    bool result = management::runtime::SetWorkingDirectory(dir);
    return Message().SetObject(result);
  }

  Message GetWorkingDir(ObjectMap &p) {
    return Message().SetObject(management::runtime::GetWorkingDirectory());
  }

  Message GetScriptAbsolutePath(ObjectMap &p) {
    return Message().SetObject(mgmt::runtime::GetScriptAbsolutePath());
  }

  Message GetPlatform(ObjectMap &p) {
    return Message().SetObject(kPlatformType);
  }

  Message GetFunctionPointer(ObjectMap &p) {
    auto tc = TypeChecking(
      { 
        Expect("library", kTypeIdString),
        Expect("id", kTypeIdString)
      }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);

#ifdef _WIN32
    wstring path = s2ws(p.Cast<string>("library"));
    string id = p.Cast<string>("id");
    HMODULE mod = LoadLibraryW(path.data());

    if (mod == nullptr) return Message().SetObject(int64_t(0));

    auto func = GenericFunctionPointer(GetProcAddress(mod, id.data()));
    CABIContainer result{ func };
#else
    string path = p.Cast<string>("library");
    string id = p.Cast<string>("id");
    void *mod = dlopen(path.data(), RTLD_LAZY);

    if (mod == nullptr) return Message().SetObject(int64_t(0));

    auto func = GenericFunctionPointer(dlsym(mod, id.data()));
    CABIContainer result{ func };
#endif
    return Message().SetObject(Object(result, kTypeIdFunctionPointer));
  }

  void InitConsoleComponents() {
    using management::CreateImpl;

    CreateImpl(FunctionImpl(Input, "msg", "input", kParamAutoFill).SetLimit(0));
    CreateImpl(FunctionImpl(GetChar, "", "getchar"));
    CreateImpl(FunctionImpl(Print, kStrMe, "print"));
    CreateImpl(FunctionImpl(PrintLine, kStrMe, "println"));
    CreateImpl(FunctionImpl(SystemCommand, "command", "console"));
    CreateImpl(FunctionImpl(ThreadSleep, "milliseconds", "sleep"));
    CreateImpl(FunctionImpl(SetWorkingDir, "dir", "setwd"));
    CreateImpl(FunctionImpl(GetWorkingDir, "", "getwd"));
    CreateImpl(FunctionImpl(GetScriptAbsolutePath, "", "get_script_path"));
    CreateImpl(FunctionImpl(GetPlatform, "", "get_platform"));
    CreateImpl(FunctionImpl(GetFunctionPointer, "library|id", "get_function_ptr"));
  }
}
