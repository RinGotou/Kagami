#include "machine.h"
#include "argument.h"

using namespace std;
using namespace suzu;
using namespace kagami;
using namespace kagami::trace;
using namespace minatsuki;
using Processor = ArgumentProcessor<kHeadHorizon, kJoinerEqual>;

namespace runtime {
  // Binary name from command line
  static string binary_name;

  // Load add built-in components
  void InitEmbeddedComponents() {
    InitPlainTypes();
    InitConsoleComponents();
    InitBaseTypes();
    InitContainerComponents();
#if defined(_WIN32)
    LoadSocketStuff();
#endif
#if not defined(_DISABLE_SDL_)
    InitSoundComponents();
#endif
  }
}

void StartInterpreter_Kisaragi(string path, string log_path, bool real_time_log) {
  Agent *agent = real_time_log ?
    static_cast<Agent *>(new StandardRealTimeAgent(log_path.data(), "a+")) :
    static_cast<Agent *>(new StandardCacheAgent(log_path.data(), "a+"));

  trace::InitLoggerSession(agent);
  trace::AddEvent("Script:" + path);

  DEBUG_EVENT("Your're running a copy of Kagami interpreter with debug flag!");

  KIR script_ir;
  KIRLoader loader(path, script_ir);

  if (loader.good) {
    Machine main_thread(script_ir);
    main_thread.Run();
  }

  trace::StopLoggerSession();
  delete agent;
}

void ApplicationInfo() {
  puts(ENGINE_NAME " " INTERPRETER_VER "\n");
  puts("Codename:" CODENAME "\n");
  puts("Build date:" __DATE__ "\n");
  puts(COPYRIGHT ", " MAINTAINER "\n");
}

void HelpFile() {
  puts("Usage:");
  puts(runtime::binary_name.data());
  puts(" [-OPTION][-OPTION=VALUE]...\n\n");
  puts(
    "\tpath=PATH         Path of script file.\n"
    "\tlog=(PATH|stdout) Output of error log.\n"
    "\trtlog             Enable real-time logger\n"
    "\twait              Automatically pause at application exit.\n"
    "\thelp              Show this message.\n"
    "\tversion           Show version message of interpreter.\n"
  );
}

void AtExitHandler() {
  puts("(Application Exit) Press enter to close...");
  getchar();
}

inline void Patch(string locale_str) {
  setlocale(LC_ALL, locale_str.data());
}

void Processing(Processor &processor) {
  if (processor.Exist("path")) {
    string path = processor.ValueOf("path");
    string log = processor.Exist("log") ?
      processor.ValueOf("log") :
      "project-kagami.log";
    
    if (processor.Exist("wait")) {
      atexit(AtExitHandler);
    }

    //Need to place a command argument for this option
    Patch("en_US.UTF-8");

    StartInterpreter_Kisaragi(path, log, processor.Exist("rtlog"));
  }
  else if (processor.Exist("help")) {
    HelpFile();
  }
  else if (processor.Exist("version")) {
    ApplicationInfo();
  }
}

int main(int argc, char **argv) {
  runtime::binary_name = argv[0];
  runtime::InitEmbeddedComponents();

  Processor processor = {
    Pattern("path"   , Option(true, false, 1)),
    Pattern("help"   , Option(false, false, 1)),
    Pattern("version", Option(false, false, 1)),
    Pattern("rtlog"  , Option(false, true)),
    Pattern("log"    , Option(true, true)),
    Pattern("wait"   , Option(false, true))
  };

#if not defined(_DISABLE_SDL_)
  if (dawn::EnvironmentSetup() != 0) {
    puts("SDL initialization error!");
    return 0;
  }
#endif

  if (argc <= 1) {
    HelpFile();
  }
  else {
    if (!processor.Generate(argc, argv)) {
      puts(ArgumentProcessorError(processor.Error())
        .Report(processor.BadArg()).data());
      HelpFile();
    }
    else {
      Processing(processor);
    }
  }

  
#if not defined(_DISABLE_SDL_)
  dawn::EnvironmentCleanup();
#endif
  return 0;
}
