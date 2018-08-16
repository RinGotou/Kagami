#include "kscript.h"
//#define _ENABLE_DEBUGGING_
#ifndef _NO_CUI_
#include <iostream>
#endif

namespace kagami {
  void ScriptCore::PrintEvents() {
    using namespace trace;
    ofstream ofs("event.log", std::ios::trunc);
    string priorityStr;
    if (ofs.good()) {
      if (GetLogger().empty()) {
        ofs << "No Events.\n";
      }
      else {
        for (log_t unit : GetLogger()) {
          ofs << unit.first;
          ofs << "At:" << to_string(unit.second.GetIndex()) << "\n";
          const auto value = unit.second.GetValue();
          if (value == kStrFatalError) priorityStr = "Fatal:";
          else if (value == kStrWarning) priorityStr = "Warning:";
          if (unit.second.GetDetail() != kStrEmpty) {
            ofs << priorityStr << unit.second.GetDetail() << "\n";
          }
        }
      }
    }
    ofs.close();
  }

  Message ScriptCore::ExecScriptFile(string target) {
    Message result;
    ScriptMachine machine(target.c_str());

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
    ScriptMachine machine;
    isTerminal = true;

    std::cout << kEngineName
      << ' ' << "verison:" << kEngineVersion
      << '(' << kInsideName << ')' << std::endl;
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
  scriptCore.ExecScriptFile("C:\\workspace\\ErrorTest.kagami");
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