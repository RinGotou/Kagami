#pragma once
#include "machine_kisaragi.h"

namespace kagami {
  template <int base>
  Message DecimalConvert(ObjectMap &p) {
    EXPECT_TYPE(p, "str", kTypeIdString);
    string str = ParseRawString(p["str"].Cast<string>());

    long dest = stol(str, nullptr, base);
    return Message().SetObject(Object(
      make_shared<long>(dest), kTypeIdInt
    ));
  }
}
