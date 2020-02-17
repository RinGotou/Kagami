#include "trace.h"

namespace kagami {
  //TODO:Remove index storage in Message class

  void AppendMessage(string msg, StateLevel level, 
    StandardLogger *std_logger, size_t index) {
    string buf;

    switch (level) {
    case kStateError:  buf.append("Error:"); break;
    case kStateWarning:buf.append("Warning:"); break;
    case kStateNormal: buf.append("Info:"); break;
    default:break;
    }

    buf.append(msg);
    std_logger->WriteLine(buf);
  }

  void AppendMessage(string msg, StandardLogger *std_logger) {
    std_logger->WriteLine(msg);
  }
}