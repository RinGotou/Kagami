#include "machine.h"
#include "argument.h"

using namespace std;
using namespace suzu;
using namespace kagami;
using namespace kagami::management;
using namespace minatsuki;
using Processor = ArgumentProcessor<kHeadHorizon, kJoinerEqual>;

void StartInterpreter_Kisaragi(string path, string log_path, bool real_time_log) {
  Agent *agent = real_time_log ?
    static_cast<Agent *>(new StandardRealTimeAgent(log_path.data(), "a+")) :
    static_cast<Agent *>(new StandardCacheAgent(log_path.data(), "a+"));

  trace::InitLoggerSession(agent);
  trace::AddEvent("Script:" + path);

  DEBUG_EVENT("Your're running a copy of Kagami Project Core with debugging flag!");

  VMCode &script_file = script::AppendBlankScript(path);
  VMCodeFactory factory(path, script_file);

  if (factory.Start()) {
    Machine main_thread(script_file);
    main_thread.Run();
  }

  trace::StopLoggerSession();
  delete agent;
}

void ApplicationInfo() {
  printf(ENGINE_NAME " " INTERPRETER_VER "\n");
  printf("Codename:" CODENAME "\n");
  printf("Build date:" __DATE__ "\n");
  printf(COPYRIGHT ", " MAINTAINER "\n");
}

void HelpFile() {
  printf("Usage:");
  printf("%s", runtime::GetBinaryName().data());
  printf(" [-OPTION][-OPTION=VALUE]...\n\n");
  printf(
    "\tscript=FILE         Path of script file.\n"
    "\tlog=(FILE|stdout)   Output of error log.\n"
    "\tlocale=LOCALE_STR   Locale string for interpreter.(default=en_US.UTF8)\n"
    "\tvm_stdout=FILE      Redirection of script standard output.\n"
    "\tvm_stdin=FILE       Redirection of script standard input.\n"
    "\trtlog               Enable real-time logger\n"
    "\twait                Automatically pause at application exit.\n"
    "\thelp                Show this message.\n"
    "\tversion             Show version message of interpreter.\n"
  );
}

void Motto() {
  puts("\"Praying for next morning.\"");
  puts(ENGINE_NAME " Version " INTERPRETER_VER " '"  CODENAME "'");
}

void AtExitHandler() {
  puts("(Application Exit) Press enter to close...");
  getchar();
}

void Processing(Processor &processor) {
  if (processor.Exist("script")) {
    string path = processor.ValueOf("script");
    string log = processor.Exist("log") ?
      processor.ValueOf("log") :
      "project-kagami.log";
    
    if (processor.Exist("wait")) {
      atexit(AtExitHandler);
    }

    if (processor.Exist("vm_stdout")) {
      string vm_stdout = processor.ValueOf("vm_stdout");
      if (log == vm_stdout) {
        puts("VM stdout/Log output confliction!");
        return;
      }

      GetVMStdout(fopen(vm_stdout.data(), "w"));
    }

    if (processor.Exist("vm_stdin")) {
      string vm_stdin = processor.ValueOf("vm_stdin");
      GetVMStdin(fopen(vm_stdin.data(), "r"));
    }

    setlocale(LC_ALL, processor.Exist("locale") ?
      processor.ValueOf("locale").data() : "en_US.UTF8");

    StartInterpreter_Kisaragi(path, log, processor.Exist("rtlog"));
    CloseStream();
  }
  else if (processor.Exist("help")) {
    HelpFile();
  }
  else if (processor.Exist("version")) {
    ApplicationInfo();
  }
  else if (processor.Exist("motto")) {
    Motto();
  }
}

int main(int argc, char **argv) {
  runtime::InformBinaryPathAndName(argv[0]);
  ActivateComponents();

  Processor processor = {
    Pattern("script" , Option(true, false, 1)),
    Pattern("help"   , Option(false, false, 1)),
    Pattern("version", Option(false, false, 1)),
    Pattern("motto"  , Option(false, false, 1)),
    Pattern("rtlog"  , Option(false, true)),
    Pattern("log"    , Option(true, true)),
    Pattern("wait"   , Option(false, true)),
    Pattern("locale" , Option(true, true)),
    Pattern("vm_stdout" ,Option(true, true)),
    Pattern("vm_stdin"  ,Option(true, true))
  };

  if (dawn::EnvironmentSetup() != 0) {
    puts("SDL initialization error!");
    return 0;
  }

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

  
  dawn::EnvironmentCleanup();
  return 0;
}
