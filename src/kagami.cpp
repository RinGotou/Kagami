//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Suzu Nakamura
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//*Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include "kagami.h"
//#define _ENABLE_DEBUGGING_
//#define _NO_CUI_
#ifndef _NO_CUI_
#include <iostream>
#endif

namespace kagami {
  Message Quit(ObjectMap &p) {
    Message result(kStrEmpty, kCodeQuit, kStrEmpty);
    return result;
  }

  void ScriptCore::PrintEvents() {
    using namespace trace;
    ofstream ofs("event.log", std::ios::trunc);
    string prioritystr;
    if (ofs.good()) {
      if (GetLogger().empty()) {
        ofs << "No Events.\n";
      }
      else {
        for (log_t unit : GetLogger()) {
          ofs << "[" << unit.first << "]";
          if (unit.second.GetValue() == kStrFatalError) prioritystr = "Fatal:";
          else if (unit.second.GetValue() == kStrWarning) prioritystr = "Warning:";
          if (unit.second.GetDetail() != kStrEmpty) {
            ofs << prioritystr << unit.second.GetDetail() << "\n";
          }
        }
      }
    }
    ofs.close();
  }

  Message ScriptCore::ExecScriptFile(string target) {
    Message result;
    ScriptProvider sp(target.c_str());
    ChainStorage cs(sp);

    if (target == kStrEmpty) {
      trace::log(result.combo(kStrFatalError, kCodeIllegalArgs, "Empty path string."));
      return result;
    }
    Activiate();
    cs.Run();
    entry::ResetPlugin();
    return result;
  }

#ifndef _NO_CUI_
  void ScriptCore::Terminal() {
    using namespace entry;
    string buf = kStrEmpty;
    Message result(kStrEmpty, kCodeSuccess, kStrEmpty);
    Processor loader;
    //auto Build = [&](string target) {return BuildStringVector(target); };
    std::cout << kEngineName << ' ' << kEngineVersion << std::endl;
    std::cout << kCopyright << ' ' << kEngineAuthor << std::endl;

    CreateManager();
    Activiate();
    Inject(EntryProvider(ActivityTemplate().set("quit", Quit, kFlagNormalEntry, kCodeNormalArgs, "")));

    while (result.GetCode() != kCodeQuit) {
      std::cout << ">>>";
      std::getline(std::cin, buf);
      if (buf != kStrEmpty) {
        result = loader.Reset().Build(buf).Start();
        if (result.GetCode() < kCodeSuccess) {
          std::cout << result.GetDetail() << std::endl;
        }
      }
    }
    ResetPlugin();
  }
#endif
}

int main(int argc, char **argv) {

  kagami::ScriptCore scriptCore;
#ifdef _ENABLE_DEBUGGING_
  scriptCore.ExecScriptFile("C:\\workspace\\test.kagami");
#else
#ifndef _NO_CUI_
  if (argc > 1) {
    scriptCore.ExecScriptFile(argv[1]);
  }
  else {
    scriptCore.Terminal();
  }
#else
  scriptcore.ExecScriptFile(argv[1]);
#endif
#endif
  scriptCore.PrintEvents();

  return 0;
}