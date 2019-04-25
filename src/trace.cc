#include "trace.h"

namespace kagami {
  namespace trace {
    Agent *ContactLogger(Agent *agent = nullptr) {
      static Agent *local_agent = nullptr;

      if (agent != nullptr) local_agent = agent;

      return local_agent;
    }

    void InitLoggerSession(Agent *agent) {
      ContactLogger(agent);
      agent->WriteLine("Kagami Project Session - Start");
    }

    void StopLoggerSession() {
      ContactLogger()->WriteLine("Kagami Project Session - Stop");
    }

    void AddEvent(Message msg) {
      auto *agent = ContactLogger();
      string buf;
      auto index = msg.GetIndex();

      if (msg.GetLevel() != kStateNormal && index != 0) {
        buf.append("(Line:" + to_string(index) + ")");
      }

      switch (msg.GetLevel()) {
      case kStateError:  buf.append("Error:"); break;
      case kStateWarning:buf.append("Warning:"); break;
      case kStateNormal: buf.append("Info:"); break;
      default:break;
      }

      buf.append(msg.GetDetail());
      agent->WriteLine(buf);
    }

    void AddEvent(StateCode code, string detail, StateLevel level) {
      auto *agent = ContactLogger();
      string buf;

      switch (level) {
      case kStateError:  buf.append("Error:"); break;
      case kStateWarning:buf.append("Warning:"); break;
      case kStateNormal: buf.append("Info:"); break;
      default:break;
      }

      buf.append(detail);
      agent->WriteLine(buf);
    }

    void AddEvent(string info) {
      auto *agent = ContactLogger();
      agent->WriteLine(info);
    }
  }
}