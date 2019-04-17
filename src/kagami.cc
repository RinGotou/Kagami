#include "machine.h"
#include "argument.h"

//Argument Processor
using suzu::ArgumentProcessor;
using suzu::Option;
using suzu::ArgumentProcessorError;
using suzu::Pattern;
using Processor = ArgumentProcessor<suzu::kHeadHorizon, suzu::kJoinerEquals>;

//STL
using std::string;
using std::cout;
using std::endl;
using std::cin;

using namespace kagami;

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
  trace::LoggerPolicy *logger = nullptr;

  if (real_time_log) {
    logger = new trace::RealTimePolicy();
  }
  else {
    logger = new trace::CachePolicy();
  }

  logger->Inject(log_path, path);

  trace::InitLogger(logger);
  trace::AddEvent("Interpreter start");

  DEBUG_EVENT("Your're running a copy of Kagami interpreter with debug flag!");

  IRLoader loader(path.c_str());

  if (loader.health) {
    KIR ir = loader.output;
    loader.output.clear();
    loader.output.shrink_to_fit();
    Machine main_thread(ir);
    
    main_thread.Run();
  }

  trace::AddEvent("Interpreter exit");
  logger->Final();
  delete logger;
}

void ApplicationInfo() {
  cout << kEngineName << " " << kInterpreterVersion << "\n";
  cout << "Patch: " << kPatchName << "\n";
  cout << "Build date: " << __DATE__ << "\n";
  cout << kCopyright << ", " << kMaintainer << endl;
}

void HelpFile() {
  cout << "Usage:"+ runtime::binary_name +" [-OPTION][-OPTION=VALUE]...\n\n";
  cout << "\tpath=PATH         Path of script file.\n";
  cout << "\tlog=(PATH|stdout) Output of error log.\n";
  cout << "\trtlog             Enable real-time logger\n";
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
  std::locale::global(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
  std::wcout.imbue(std::locale(locale));
  std::cout.imbue(std::locale(locale));
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
    Patch("");

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

  if (argc == 1) HelpFile();

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
    cout << "SDL initialization error!" << endl;
    return 0;
  }
#endif

  if (!processor.Generate(argc, argv)) {
    cout << ArgumentProcessorError(processor.Error()).Report(processor.BadArg()) 
      << endl;
    HelpFile();
  }
  else {
    Processing(processor);
  }
  
#if not defined(_DISABLE_SDL_)
  dawn::EnvironmentCleanup();
#endif
  return 0;
}
