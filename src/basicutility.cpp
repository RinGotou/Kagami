#include "basicutility.h"

namespace Entry {
  using namespace Suzu;
  deque<MemoryProvider> childbase;
}

namespace Suzu {

  Message CommaExpression(vector<string> &res) {
    Message result;
    if (res.empty()) result.SetCode(kCodeRedirect).SetValue(kStrEmpty);
    else result.SetCode(kCodeRedirect).SetValue(res.back());
    return result;
  }

  Message MemoryQuery(vector<string> &res) {
    using namespace Entry;
    Message result;
    string temp;
    size_t begin = childbase.size() - 1;
    size_t i = 0;

    if (childbase.empty()) {
      Tracking::log(result.SetValue(kStrWarning)
        .SetCode(kCodeIllegalArgs)
        .SetDetail("MemoryQuery() 1"));
    }
    else {
      if (childbase.size() == 1 || res[1] == kArgOnce) {
        temp = childbase.back().query(res[0]);
      }
      else if (childbase.size() > 1 && res[1] != kArgOnce) {
        for (i = begin; i >= 0; --i) {
          temp = childbase[i].query(res[0]);
          if (temp != kStrNull) break;
        }
      }

      if (temp != kStrNull) {
        result.SetCode(kCodeSuccess).SetValue(kStrSuccess).SetDetail(temp);
      }
      else {
        Tracking::log(result.SetCode(kCodeIllegalCall)
          .SetValue(kStrFatalError)
          .SetDetail("MemoryQuery() 2"));
      }
    }

    return result;
  }

  Message Calculate(vector<string> &res) {
    Message result;

    //pending

    return result;
  }

  void TotalInjection() {
    using namespace Entry;
    //set root memory provider
    childbase.push_back(MemoryProvider());
    childbase.back().SetParent(&(childbase.back()));
    //inject basic Entry provider
    Inject(EntryProvider("commaexp", CommaExpression, kFlagAutoSize));
    Inject(EntryProvider("memquery", MemoryQuery, 2, kFlagCoreEntry));
    Inject(EntryProvider("calculat", Calculate, kFlagAutoSize, kFlagCoreEntry));

  }
}