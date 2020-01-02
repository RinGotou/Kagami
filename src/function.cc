#include "function.h"

namespace kagami {
  Message MakeInvokePoint(string id, string type_id) {
    vector<string> arg = { id,type_id };
    return Message(util::CombineStringVector(arg))
      .SetInvokingSign();
  }
}