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
      string str(nowtime);
      str.pop_back();
      GetLogger().emplace_back(log_t(str, msg));
#else
      string nowtime(ctime(&now));
      nowtime.pop_back();
      GetLogger().emplace_back(log_t(nowtime, msg));
#endif
    }
  }
}