#include "machine.h"

namespace kagami {
  Message StructGetMembers(ObjectMap &p) {
    auto &struct_def = p.Cast<ObjectStruct>(kStrMe);
    auto managed_array = make_shared<ObjectArray>();

    for (auto &unit : struct_def.GetContent()) {
      managed_array->push_back(Object(unit.first));
    }

    return Message().SetObject(Object(managed_array, kTypeIdArray));
  }

  void InitStructComponents() {
    using namespace mgmt;
    using namespace mgmt::type;

    ObjectTraitsSetup(kTypeIdStruct, ShallowDelivery)
      .InitMethods({
        FunctionImpl(StructGetMembers, "", "members")
      });

    EXPORT_CONSTANT(kTypeIdStruct);
  }
}