#pragma once
#include "vmcode.h"
 
namespace kagami {
  namespace trace {
    void InitLoggerSession(Agent *agent);
    void StopLoggerSession();
    void AddEvent(Message msg);
    void AddEvent(string detail, StateLevel level);
    void AddEvent(string info);
  }
}

