#pragma once
#include "machine.h"

using std::cout;
using std::endl;
using std::cin;

namespace kagami {
  class ScriptCore {
  private:
    template <class T>
    void LogOutput(T &stream, const char *path, const char *scriptPath) {
      using namespace trace;
      string priority;
      auto &logger = GetLogger();
      
      if (!logger.empty()) {
        stream << "[Script:" << scriptPath << "]" << endl;
      } 
      for (log_t unit : GetLogger()) {
        //time
        stream << "[" << unit.first << "]";
        //line
        stream << "(Line:" << to_string(unit.second.GetIndex() + 1) << ")";
        //message string
        const auto value = unit.second.GetValue();
        if (value == kStrFatalError) priority = "Fatal:";
        else if (value == kStrWarning) priority = "Warning:";
        if (unit.second.GetDetail() != kStrEmpty) {
          stream << priority << unit.second.GetDetail() << endl;
        }
      }
    }
  public:
    void PrintEvents(const char *path, const char *scriptPath);
    void ExecScriptFile(string target);
    void MyInfo();
  };
}