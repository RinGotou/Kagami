#pragma once
#include "module.h"

using std::cout;
using std::endl;
using std::cin;

namespace kagami {
  class ScriptCore {
  private:
    template <class T>
    void LogOutput(T &stream, const char *path, const char *script_path) {
      using namespace trace;
      string priority;
      auto &logger = GetLogger();
      
      if (!logger.empty()) {
        stream << "[Script:" << script_path << "]" << endl;
      } 

      for (log_t unit : GetLogger()) {
        //time
        stream << "[" << unit.first << "]";
        //line
        stream << "(Line:" << to_string(unit.second.GetIndex() + 1) << ")";
        //message string
        switch (unit.second.GetLevel()) {
        case kStateError:priority = "Fatal:"; break;
        case kStateWarning:priority = "Warning:"; break;
        }
        if (unit.second.GetDetail() != "") {
          stream << priority << unit.second.GetDetail() << endl;
        }
      }
    }
  public:
    void PrintEvents(string path, string script_path);
    void ExecScriptFile(string target);
    void MyInfo();
  };
}