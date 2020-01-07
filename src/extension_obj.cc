#include "machine.h"

namespace kagami {
  Message NewExtension(ObjectMap &p) { 
    auto tc = TypeChecking({ Expect("path",kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);
    auto &path = p.Cast<string>("path");
    ManagedExtension extension = make_shared<Extension>(path);
    return Message().SetObject(Object(extension, kTypeIdExtension));
  }

  Message ExtensionGood(ObjectMap &p) {
    auto &extension = p.Cast<Extension>(kStrMe);
    return Message().SetObject(extension.Good());
  }

  Message ExtensionFetchFunction(ObjectMap &p) {

  }

  void InitExtensionComponents() {
    //Reserved
  }
}