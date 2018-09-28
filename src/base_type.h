#pragma once
#include "machine.h"

namespace kagami {
  const string kArrayBaseMethods = "size|__at|__print";
  const string kStringMethods = "size|__at|__print|substr|to_wide";
  const string kWideStringMethods = "size|__at|__print|substr|to_byte";
  const string kInStreamMethods = "get|good|getlines|close|eof";
  const string kOutStreamMethods = "write|good|close";
  const string kRegexMethods = "match";

  using ArrayBase = vector<Object>;
}