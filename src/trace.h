#pragma once
#include "analyzer.h"
 
namespace kagami {
  namespace trace {
    void InitLoggerSession(Agent *agent);
    void StopLoggerSession();
    void AddEvent(Message msg);
    void AddEvent(StateCode code, string detail, StateLevel level = kStateNormal);
    void AddEvent(string info);
  }
}

#if defined(_DEBUG_)
#define DEBUG_EVENT(MSG) trace::AddEvent(MSG)
#define ASSERT_MSG(COND, MSG)               \
  if (COND) { DEBUG_EVENT(MSG); }
#else
#define DEBUG_EVENT(MSG)
#define ASSERT_MSG(COND, MSG)
#endif

