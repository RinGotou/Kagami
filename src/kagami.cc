#include "machine.h"
#include "argument.h"

using namespace std;
using namespace kagami;
using namespace kagami::management;
using namespace minatsuki;

using Processor = ArgumentProcessor<kHeadHorizon, kJoinerEqual>;

void BootMainVMObject(string path, string log_path, bool real_time_log) {
  VMCode &script_file = script::AppendBlankScript(path);

  {
    VMCodeFactory factory(path, script_file, log_path, real_time_log);
    if (!factory.Start()) return;
  }
  
  Machine main_thread(script_file, log_path, real_time_log);
  main_thread.Run();
}

void ApplicationInfo() {
  printf(PRODUCT " " PRODUCT_VER "\n");
  printf("Codename:" CODENAME "\n");
  printf("Build date:" __DATE__ "\n");
  printf("Dawn Version:" DAWN_VERSION "\n");
  printf(COPYRIGHT ", " AUTHOR "\n");
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
  puts(PRODUCT "\nVersion " PRODUCT_VER " '"  CODENAME "'");
}

void Processing(Processor &processor) {
  if (processor.Exist("script")) {
    string path = processor.ValueOf("script");
    string log = processor.Exist("log") ?
      processor.ValueOf("log") :
      "project-kagami.log";

    if (processor.Exist("vm_stdout")) {
      string vm_stdout = processor.ValueOf("vm_stdout");
      if (log == vm_stdout) {
        puts("VM stdout/log output confliction");
        return;
      }

      GetVMStdout(fopen(vm_stdout.data(), "a"));
    }

    if (processor.Exist("vm_stdin")) {
      string vm_stdin = processor.ValueOf("vm_stdin");
      if (log == vm_stdin) {
        puts("VM stdin/log output confliction");
        return;
      }


      GetVMStdin(fopen(vm_stdin.data(), "r"));
    }

    setlocale(LC_ALL, processor.Exist("locale") ?
      processor.ValueOf("locale").data() : "en_US.UTF8");

    runtime::InformScriptPath(path);
    BootMainVMObject(path, log, processor.Exist("rtlog"));
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

void InitFromConfigFile() {
  try {
    auto file = toml::parse("init.toml");
    auto startup = toml::find(file, "Startup");

    auto script = toml::find<string>(startup, "script");
    auto log = [&startup]() -> string {
      auto temp = toml::expect<string>(startup, "log");
      if (temp.is_ok()) return temp.unwrap();
      return "project-kagami.log";
    }();
    auto real_time_log = toml::expect<bool>(startup, "real_time_log");
    auto locale = toml::expect<string>(startup, "locale");
    auto vm_stdin = toml::expect<string>(startup, "vm_stdin");
    auto vm_stdout = toml::expect<string>(startup, "vm_stdout");

    if (vm_stdout.is_ok()) {
      if (log == vm_stdout.unwrap()) {
        puts("VM stdout/log output confliction");
        return;
      }

      GetVMStdout(fopen(vm_stdout.unwrap().data(), "a"));
    }

    if (vm_stdin.is_ok()) {
      if (log == vm_stdin.unwrap()) {
        puts("VM stdin/log output confliction");
      }

      GetVMStdin(fopen(vm_stdin.unwrap().data(), "r"));
    }

    setlocale(LC_ALL, locale.is_ok() ? locale.unwrap().data() : "en_US.UTF8");

    runtime::InformScriptPath(script);
    BootMainVMObject(script, log, real_time_log.is_ok() ?
      real_time_log.unwrap() : false);
    CloseStream();
  }
  catch (std::runtime_error &e) {
    printf(e.what());
  }
  catch (toml::syntax_error &e) {
    printf(e.what());
  }
  catch (toml::type_error &e) {
    printf(e.what());
  }
}

int main(int argc, char **argv) {
  namespace fs = std::filesystem;
  runtime::InformBinaryPathAndName(argv[0]);
  ActivateComponents();

  if (fs::exists(fs::path("init.toml"))) {
    InitFromConfigFile();
    return 0;
  }

  Processor processor = {
    Pattern("script" , Option(true, false, 1)),
    Pattern("help"   , Option(false, false, 1)),
    Pattern("version", Option(false, false, 1)),
    Pattern("motto"  , Option(false, false, 1)),
    Pattern("rtlog"  , Option(false, true)),
    Pattern("log"    , Option(true, true)),
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
