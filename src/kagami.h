#pragma once
#include "parser.h"

namespace kagami {
  /*Core Class
  this class contains all main functions of script processor.
  */
  class ScriptCore {
  private:
    bool ignoreFatalError;
  public:
    //TODO:startup arugment
    void Terminal() const;
    static void PrintEvents();
    static Message ExecScriptFile(string target);
  };
}