#pragma once
#include "machine.h"

namespace kagami {
  /*Core Class
  this class contains all main functions of script processor.
  */
  class ScriptCore {
  private:
    bool ignoreFatalError;
    bool isTerminal;
  public:
    void PrintEvents();
    Message ExecScriptFile(string target);
#ifndef _NO_CUI_
    void Terminal2();
#endif
  };
}