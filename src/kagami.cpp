#include "kagami.h"
#include "suzu_ap.h"

using suzu::ArgumentProcessor;
using suzu::Option;
using suzu::ArgumentProcessorError;
using suzu::Pattern;
using std::string;
using std::map;
using std::vector;
using Processor = ArgumentProcessor<suzu::kHeadHorizon, suzu::kJoinerEquals>;

namespace kagami {
  void ScriptCore::PrintEvents(string path, string script_path) {
    using namespace trace;
    string priorityStr;
    auto logger = GetLogger();

    /*Write log to a file*/
    if (path != "stdout" && path != "") {
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
    IRMaker maker(target.c_str());
    Module main_module(maker, true);
    Activiate();

    if (main_module.Good()) {
      main_module.Run();
    }
  }

  void ScriptCore::MyInfo() {
    cout << kEngineName << " " << kInterpreterVersion << "\n";
    cout << "IR Framework Version: " << kIRFrameworkVersion << "\n";
    cout << "Patch: " << kPatchName << "\n";
    cout << kCopyright << " " << kMaintainer << endl;
  }
}

//Main namespace
void HelpFile() {
  cout << "Usage:kagami [-OPTION][-OPTION=VALUE]...\n\n";
  cout << "\tpath=PATH         Path of script file.\n";
  cout << "\tlog=(PATH|stdout) Output of error log.\n";
  cout << "\twait              Automatically pause at application exit.\n";
  cout << "\thelp              Show this message.\n";
  cout << "\tversion           Show version message of interpreter.\n";

}

void AtExitHandler() {
  cout << "(Application Exit) Press enter to close..." << endl;
  cin.get();
}

inline void Patch(string locale) {
  std::ios::sync_with_stdio(false);
  //solve utf-8 encoding
  //Although codecvt_utf8 is not available in C++17..
  //But we're now in C++11,isn't it?
  std::locale::global(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
  std::wcout.imbue(std::locale(locale));
}

void Processing(Processor &processor) {
  kagami::ScriptCore core;

  if (processor.Exist("path")) {
    string path = processor.ValueOf("path");
    string log = processor.Exist("log") ?
      processor.ValueOf("log") :
      "project-kagami.log";
    
    if (processor.Exist("wait")) {
      atexit(AtExitHandler);
    }

    Patch("");

    core.ExecScriptFile(path);
    core.PrintEvents(log, path);
  }
  else if (processor.Exist("help")) {
    HelpFile();
  }
  else if (processor.Exist("version")) {
    core.MyInfo();
  }
}

int main(int argc, char **argv) {
#if not defined(_DISABLE_SDL_)
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
    cout << "SDL initialization error!" << endl;
    return 0;
  }
#endif
  
  Processor processor = {
    Pattern("path", Option(true, false, 1)),
    Pattern("help", Option(false, false, 1)),
    Pattern("version", Option(false, false, 1)),
    Pattern("log", Option(true, true)),
    Pattern("wait", Option(false, true))
  };

  if (!processor.Generate(argc, argv)) {
    cout << 
      ArgumentProcessorError(processor.Error())
      .Report(processor.BadArg()) 
      << endl;
    HelpFile();
  }
  else {
    Processing(processor);
  }
  
#if not defined(_DISABLE_SDL_)
  SDL_Quit();
#endif
  return 0;
}