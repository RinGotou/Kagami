#include "trace.h"

namespace kagami {
  namespace trace {
    LoggerPolicy *ContactLogger(LoggerPolicy *policy = nullptr) {
      static LoggerPolicy *logger_policy = nullptr;

      if (policy != nullptr) {
        logger_policy = policy;
      }

      return logger_policy;
    }

    void InitLogger(LoggerPolicy *policy) {
      ContactLogger(policy)->Init();
    }

    void AddEvent(Message msg) {
      auto *logger = ContactLogger();
      logger->Add(msg);
    }

    void AddEvent(StateCode code, string detail, StateLevel level) {
      auto *logger = ContactLogger();
      logger->Add(Message(code, detail, level));
    }

    void AddEvent(string info) {
      Message msg(kCodeSuccess, info);
      
      auto *logger = ContactLogger();
      logger->Add(msg);
    }
  }
}