#pragma once
#include "analyzer.h"
#include "processor.h"

namespace kagami {
  namespace trace {
    using log_t = pair<string, Message>;
    void Log(kagami::Message msg);
    vector<log_t> &GetLogger();
  }
}