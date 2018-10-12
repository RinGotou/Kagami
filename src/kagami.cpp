#include "kagami.h"
#include <iostream>

namespace kagami {
  void ScriptCore::PrintEvents(const char *path, const char *scriptPath) {
    using namespace trace;
    string priorityStr;
    auto logger = GetLogger();

    /*Write log to a file*/
    if (path != nullptr) {
      ofstream ofs;
      ofs.open(path, std::ios::out | std::ios::app);
      if (!ofs.good()) {
        cout << "Cannot create event log.exit." << endl;
        return;
      }
      LogOutput<ofstream>(ofs, path, scriptPath);
      ofs.close();
    }
    /*Print log to screen*/
    else {
      LogOutput<std::ostream>(cout, path, scriptPath);
    }
  }

  void ScriptCore::ExecScriptFile(string target) {
    Activiate();
    Machine machine(target.c_str());
    machine.Run();
  }

  void ScriptCore::MyInfo() {
    //print application info
    cout << kEngineName
      << ' ' << "verison:" << kEngineVersion
      << '(' << kCodeName << ')' << endl;
    cout << kCopyright << ' ' << kEngineAuthor << endl;
  }
}

  void HelpFile() {
    cout << "\nargument with '*': you can leave it blank."
      << endl;
    cout << "run   [script-path][*log-path] Open a script file to execute;\n"
      << "\t[script-path] Directory path of Kagami script file;\n"
      << "\t[log-path] Directory path of event log file.\n"
      << "\thint:if user doesn't provide a log-path, application will write into 'project-kagami.log' by default."
      << "\n" << endl;
    cout << "runs  [script-path][*log-path] Same as 'run' but pause at application exit."
      << "\n" << endl;
    cout << "runp  [script-path] Same as 'run' but event info will print to standard output directly."
      << "\n" << endl;
    cout << "help  Show this message."
      << "\n" << endl;
  }

  int GetOption(char *src) {
    int res = -1;
    if (strcmp(src, "run") == 0) res = 1;
    else if (strcmp(src, "runs") == 0) res = 2;
    else if (strcmp(src, "runp") == 0) res = 3;
    else if (strcmp(src, "help") == 0) res = 4;
    return res;
  }

#ifndef _NO_CUI_
  void AtExitHandler() {
    cout << "Press enter to close..." << endl;
    cin.get();
  }
#endif

int main(int argc, char **argv) {
  kagami::ScriptCore scriptCore;

  std::ios::sync_with_stdio(false);
  //solve utf-8 encoding
  //Although codecvt_utf8 is not available in C++17..
  //But we're now in C++11,isn't it?
  std::locale::global(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
  std::wcout.imbue(std::locale(""));

  //switch main code between test case and normal case.
  //this macro can be found in the head of this file.
#ifdef _ENABLE_DEBUGGING_
  //set your own test script path here
  scriptCore.ExecScriptFile("C:\\workspace\\test.kagami");
  scriptCore.PrintEvents(nullptr, "C:\\workspace\\test.kagami");
#else
  if (argc < 2) {
    scriptCore.MyInfo();
    HelpFile();
  }
  else {
    int op = GetOption(argv[1]);
    switch (op) {
    case 1:
    case 2:
    case 3:
      if (argc < 3) {
        scriptCore.MyInfo();
        cout << "You must provide a script path.exit." << endl;
      }
      else {
        if (op == 2) atexit(AtExitHandler);
        scriptCore.ExecScriptFile(argv[2]);

        if (op == 3) {
          scriptCore.PrintEvents(nullptr, argv[2]);
        }
        else if (argc < 4) {
          scriptCore.PrintEvents("project-kagami.log", argv[2]);
        }
        else if (argc == 4) {
          scriptCore.PrintEvents(argv[3], argv[2]);
        }
      }
      break;
    case 4:
      scriptCore.MyInfo();
      HelpFile();
    case -1:
    default:
      scriptCore.MyInfo();
      cout << "Unknown option.exit." << endl;
      break;
    }
  }
#endif
  
  return 0;
}