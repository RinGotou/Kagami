#include "rebornkit.h"

namespace Suzu {
  MsgValue Message::Export() {
    MsgValue result;
    result.value = (char *)malloc(value.size());
    result.detail = (char *)malloc(detail.size());
    strcpy(result.value, value.data());
    strcpy(result.detail, detail.data());
    result.code = code;
    return result;
  }
}