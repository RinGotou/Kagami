#pragma once
#include "vmcode.h"
 
namespace kagami {
  void AppendMessage(Message msg, StandardLogger *std_logger);
  void AppendMessage(string msg, StateLevel level, StandardLogger *std_logger);
  void AppendMessage(string msg, StandardLogger *std_logger);

  //deprecated
  //namespace trace {
  //  void InitLoggerSession(StandardLogger *std_logger);
  //  void StopLoggerSession();
  //  void AddEvent(Message msg);
  //  void AddEvent(string detail, StateLevel level);
  //  void AddEvent(string info);
  //}
}

