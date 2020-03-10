#include "vmcode.h"

namespace kagami {
  bool VMCode::FindJumpRecord(size_t index, stack<size_t> &dest) {
    if (source_ != nullptr) return source_->FindJumpRecord(index, dest);

    bool found = false;
    while (!dest.empty()) dest.pop();

    if (auto it = jump_record_.find(index); it != jump_record_.end()) {
      for (auto rit = it->second.rbegin(); rit != it->second.rend(); ++rit) {
        dest.push(*rit);
      }

      found = true;
    }

    return found;
  }
}