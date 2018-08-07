#pragma once
#include "parser.h"

namespace kagami {
  /*Core Class
  this class contains all main functions of script processor.
  */
  class ScriptCore {
  private:
    bool ignoreFatalError;
    bool isTerminal;
  public:
    //TODO:startup arugment
    void Terminal();
    void PrintEvents();
    Message ExecScriptFile(string target);
  };
}