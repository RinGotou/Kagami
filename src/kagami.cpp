#include "kagami.h"
//#define _ENABLE_DEBUGGING_
#ifndef _NO_CUI_
#include <iostream>
#endif

namespace kagami {
  void ScriptCore::PrintEvents() {
    using namespace trace;
    string priorityStr;
    if (!GetLogger().empty()) {
      std::cout << "Tracking:\n";
    }
    for (log_t unit : GetLogger()) {
      std::cout << unit.first;
      std::cout << "At:" << to_string(unit.second.GetIndex() + 1) << "\n";
      const auto value = unit.second.GetValue();
      if (value == kStrFatalError) priorityStr = "Fatal:";
      else if (value == kStrWarning) priorityStr = "Warning:";
      if (unit.second.GetDetail() != kStrEmpty) {
        std::cout << priorityStr << unit.second.GetDetail() << "\n";
      }
    }

  }

  Message ScriptCore::ExecScriptFile(string target) {
    Message result;
    Machine machine(target.c_str());

    isTerminal = false;

    if (target == kStrEmpty) {
      trace::Log(result.combo(kStrFatalError, kCodeIllegalParm, "Empty path string."));
      return result;
    }
    Activiate();
    machine.Run();
    return result;
  }

#ifndef _NO_CUI_
  void ScriptCore::Terminal2() {
    Machine machine;
    isTerminal = true;

    std::cout << kEngineName
      << ' ' << "verison:" << kEngineVersion
      << '(' << kCodeName << ')' << std::endl;
    std::cout << kCopyright << ' ' << kEngineAuthor << std::endl;

    Activiate();
    machine.Terminal();
  }
#endif
}

//#ifndef _NO_CUI_
//void AtExitHandler() {
//  std::cout << "Press enter to close..." << std::endl;
//  std::cin.get();
//}
//#endif

int main(int argc, char **argv) {
  kagami::ScriptCore scriptCore;
  //atexit(AtExitHandler);

  //switch main code between test case and normal case.
  //this macro can be found in the head of this file.
#ifdef _ENABLE_DEBUGGING_
  auto &base = kagami::entry::GetObjectStack();
  scriptCore.ExecScriptFile("C:\\workspace\\test.kagami");
  //atexit(AtExitHandler);
#else
#ifndef _NO_CUI_
  if (argc > 1) {
    scriptCore.ExecScriptFile(argv[1]);
  }
  else {
    scriptCore.Terminal2();
  }
#else
  scriptcore.ExecScriptFile(argv[1]);
#endif
#endif
  scriptCore.PrintEvents();
  kagami::entry::DisposeManager();

  return 0;
}