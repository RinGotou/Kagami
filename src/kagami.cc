#include "module.h"
#include "suzu_ap.h"

//Argument Processor
using suzu::ArgumentProcessor;
using suzu::Option;
using suzu::ArgumentProcessorError;
using suzu::Pattern;
//STL
using std::string;
using std::cout;
using std::endl;
using std::cin;
using Processor = ArgumentProcessor<suzu::kHeadHorizon, suzu::kJoinerEquals>;

using namespace kagami;


void StartInterpreter(string path, string log_path, bool real_time_log) {
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

  IRMaker maker(path.c_str());
  Module main_module(maker, true);

  //Initializers
  Activiate();
  InitBaseTypes();
  InitContainerComponents();

#if defined(_WIN32)
  LoadSocketStuff();
#endif
#if not defined(_DISABLE_SDL_)
  LoadSDLStuff();
#endif

  if (main_module.Good()) {
    main_module.Run();
  }

  trace::AddEvent("Interpreter exit");
  logger->Final();
  delete logger;
}

void ApplicationInfo() {
  cout << kEngineName << " " << kInterpreterVersion << "\n";
  cout << "IR Framework Version: " << kIRFrameworkVersion << "\n";
  cout << "Patch: " << kPatchName << "\n";
  cout << kCopyright << ", " << kMaintainer << endl;
}

//Main namespace
void HelpFile() {
  cout << "Usage:kagami [-OPTION][-OPTION=VALUE]...\n\n";
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
  //solve utf-8 encoding
  //Although codecvt_utf8 is not available in C++17..
  //But we're now in C++11,isn't it?
  std::locale::global(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>));
  std::wcout.imbue(std::locale(locale));
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

    Patch("");

    StartInterpreter(path, log, processor.Exist("rtlog"));
  }
  else if (processor.Exist("help")) {
    HelpFile();
  }
  else if (processor.Exist("version")) {
    ApplicationInfo();
  }
}

int main(int argc, char **argv) {
  Processor processor = {
    Pattern("path"   , Option(true, false, 1)),
    Pattern("help"   , Option(false, false, 1)),
    Pattern("version", Option(false, false, 1)),
    Pattern("rtlog"  , Option(false, true)),
    Pattern("log"    , Option(true, true)),
    Pattern("wait"   , Option(false, true))
  };

#if not defined(_DISABLE_SDL_)
  if (SDL_Init(SDL_INIT_EVERYTHING) < 0) {
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
  SDL_Quit();
#endif
  return 0;
}