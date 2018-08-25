#include "trace.h"

namespace kagami {
  namespace trace {
    vector<log_t> &GetLogger() {
      static vector<log_t> base;
      return base;
    }

    void Log(Message msg) {
      auto now = time(nullptr);
#if defined(_WIN32) && defined(_MSC_VER)
      char nowtime[30] = { ' ' };
      ctime_s(nowtime, sizeof(nowtime), &now);
      GetLogger().emplace_back(log_t(string(nowtime), msg));
#else
      string nowtime(ctime(&now));
      GetLogger().emplace_back(log_t(nowtime, msg));
#endif
    }
  }
}