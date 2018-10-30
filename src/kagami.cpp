#include "kagami.h"

using std::string;
using std::map;
using std::vector;

namespace kagami {
  void ScriptCore::PrintEvents(string path, string script_path) {
    using namespace trace;
    string priorityStr;
    auto logger = GetLogger();

    /*Write log to a file*/
    if (path != "") {
      ofstream ofs;
      ofs.open(path, std::ios::out | std::ios::app);
      if (!ofs.good()) {
        cout << "Can't create event log." << endl;
        return;
      }
      LogOutput<ofstream>(ofs, path.c_str(), script_path.c_str());
      ofs.close();
    }
    /*Print log to screen*/
    else {
      LogOutput<std::ostream>(cout, nullptr, script_path.c_str());
    }
  }

  void ScriptCore::ExecScriptFile(string target) {
    Activiate();
    Machine machine(target.c_str());
    machine.Run();
  }

  void ScriptCore::MyInfo() {
    cout << kEngineName << " " << kEngineVersion << "\n";
    cout << "Backend version: " << kBackendVerison << "\n";
    cout << kCopyright << " " << kMaintainer << endl;
  }
}

//Main namespace
void HelpFile() {
  cout << "Execute a Kagami script.\n";
  cout << "Usage:kagami [OPTION]...\n\n";
  cout << "\t-path           Path of script file.\n";
  cout << "\t-log-path       Path of error log.\n";
  cout << "\t-pause-at-exit  Automatically pause at application exit.\n";
  cout << "\t-help           Show this message.\n";
  cout << "\t-version        Show version message of hatsuki machine.\n";

}

#ifndef _NO_CUI_
void AtExitHandler() {
  cout << "(Application Exit) Press enter to close..." << endl;
  cin.get();
}
#endif

inline void Patch(string locale) {
  std::ios::sync_with_stdio(false);
  //solve utf-8 encoding
  //Although codecvt_utf8 is not available in C++17..
  //But we're now in C++11,isn't it?
  std::locale::global(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
  std::wcout.imbue(std::locale(locale));
}

inline vector<string> Generate(int argc, char **argv) {
  vector<string> result;

  for (int idx = 1; idx < argc; idx += 1) {
    result.emplace_back(string(argv[idx]));
  }

  return result;
}

bool ArgumentParser(vector<string> args, map<string,string> &arg_base) {
  bool arg = false, status = true;
  string error_arg;
  string last;

  for (const auto &unit : args) {
    if (unit[0] == '-' || unit[0] == '/') {
      arg = true;
      last = unit.substr(1, unit.size() - 1);
      arg_base[last] = "";
      continue;
    }
    else {
      if (arg) {
        arg_base[last] = unit;
        last.clear();
        arg = false;
      }
      else {
        status = false;
        error_arg = unit;
        break;
      }
    }
  }

  if (!status) {
    cout << "Invaild argument:" + error_arg << endl;
    return false;
  }

  return true;
}

void Do(map<string, string> &base) {
  kagami::ScriptCore core;

  auto check = [&base](string id)->bool {
    return base.find(id) != base.end();
  };

  if (check("path")) {
    string path = base["path"];
    string log_path = check("log-path") ?
      base["log-path"] :
      "project-kagami.log";
    string locale = check("locale") ?
      base["locale"] :
      "";

    Patch(locale);

    bool pause_at_exit = check("pause-at-exit");

    if (check("pause-at-exit")) atexit(AtExitHandler);

    if (check("help") || check("version")) {
      cout << "Ignore illegal operation..." << endl;
    }

    core.ExecScriptFile(path);
    check("log-to-stdout") ?
      core.PrintEvents("", path) :
      core.PrintEvents(log_path, path);
  }
  else if (check("help")) {
    HelpFile();

    if (check("path") || check("version")) {
      cout << "Ignore illegal operation..." << endl;
    }
  }
  else if (check("version")) {
    kagami::ScriptCore().MyInfo();

    if (check("path") || check("help")) {
      cout << "Ignore illegal operation..." << endl;
    }
  }
  else {
    cout << "Nothing to do." << endl;
  }
}

int main(int argc, char **argv) {
#if not defined(_DISABLE_SDL_)
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    cout << "SDL initialization error!" << endl;
    return 0;
  }
#endif
  
  vector<string> str = Generate(argc, argv);
  map<string, string> args;
  bool result = ArgumentParser(str, args);
  if (result) Do(args);

#if not defined(_DISABLE_SDL_)
  SDL_Quit();
#endif
  return 0;
}