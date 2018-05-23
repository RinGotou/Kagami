#include "parser.h"

namespace kagami {
  //RawString
  shared_ptr<void> StringCopy(shared_ptr<void> target) {
    string temp(*static_pointer_cast<string>(target));
    return make_shared<string>(temp);
  }

  //Array
  shared_ptr<void> ArrayCopy(shared_ptr<void> target) {
    auto ptr = static_pointer_cast<deque<Object>>(target);
    size_t size = ptr->size(), count = 0;
    deque<Object> base;

    for (auto &unit : *ptr) {
      base.push_back(unit);
    }
    return make_shared<deque<Object>>(base);
  }

  //Null
  shared_ptr<void> NullCopy(shared_ptr<void> target) {
    return nullptr;
  }

  void InitTemplates() {
    using type::AddTemplate;
    AddTemplate(kTypeIdRawString, ObjTemplate(StringCopy, "size|substr|at"));
    AddTemplate(kTypeIdArrayBase, ObjTemplate(ArrayCopy, "size|at"));
    AddTemplate(kTypeIdNull, ObjTemplate(NullCopy, ""));
  }
}