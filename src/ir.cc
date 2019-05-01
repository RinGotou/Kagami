#include "ir.h"

namespace kagami {
  bool VMCode::FindJumpRecord(size_t index, stack<size_t> &dest) {
    bool found = false;
    if (auto it = jump_record_.find(index); it != jump_record_.end()) {
      for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
        dest.push(*rit);
      }

      found = true;
    }

    return found;
  }
}