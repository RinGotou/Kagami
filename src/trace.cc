#include "trace.h"

namespace kagami {
  void AppendMessage(Message msg, StandardLogger *std_logger) {

  }

  void AppendMessage(string msg, StateLevel level, StandardLogger *std_logger) {

  }

  void AppendMessage(string msg, StandardLogger *std_logger) {

  }

  //deprecated
  //namespace trace {
  //  StandardLogger *ContactLogger(StandardLogger *std_logger = nullptr) {
  //    static StandardLogger *local_agent = nullptr;

  //    if (std_logger != nullptr) local_agent = std_logger;

  //    return local_agent;
  //  }

  //  void InitLoggerSession(StandardLogger *std_logger) {
  //    ContactLogger(std_logger);
  //    std_logger->WriteLine("Kagami Project Core Session - Start");
  //  }

  //  void StopLoggerSession() {
  //    ContactLogger()->WriteLine("Kagami Project Core Session - Stop");
  //  }

  //  void AddEvent(Message msg) {
  //    auto *std_logger = ContactLogger();
  //    string buf;
  //    auto index = msg.GetIndex();

  //    if (msg.GetLevel() != kStateNormal && index != 0) {
  //      buf.append("(Line:" + to_string(index) + ")");
  //    }

  //    switch (msg.GetLevel()) {
  //    case kStateError:  buf.append("Error:"); break;
  //    case kStateWarning:buf.append("Warning:"); break;
  //    case kStateNormal: buf.append("Info:"); break;
  //    default:break;
  //    }

  //    buf.append(msg.GetDetail());
  //    std_logger->WriteLine(buf);
  //  }

  //  void AddEvent(string detail, StateLevel level) {
  //    auto *std_logger = ContactLogger();
  //    string buf;

  //    switch (level) {
  //    case kStateError:  buf.append("Error:"); break;
  //    case kStateWarning:buf.append("Warning:"); break;
  //    case kStateNormal: buf.append("Info:"); break;
  //    default:break;
  //    }

  //    buf.append(detail);
  //    std_logger->WriteLine(buf);
  //  }

  //  void AddEvent(string info) {
  //    auto *std_logger = ContactLogger();
  //    std_logger->WriteLine(info);
  //  }
  //}
}