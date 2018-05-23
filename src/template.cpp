#include "parser.h"

namespace kagami {
  //RawString
  shared_ptr<void> StringCopy(shared_ptr<void> target) {
    string temp(*static_pointer_cast<string>(target));
    return make_shared<string>(temp);
  }

  shared_ptr<void> NullCopy(shared_ptr<void> target) {
    return nullptr;
  }

  void InitTemplates() {
    using type::AddTemplate;
    AddTemplate(kTypeIdRawString, ObjTemplate(StringCopy, "size|substr"));
    AddTemplate(kTypeIdNull, ObjTemplate(NullCopy, ""));
  }
}