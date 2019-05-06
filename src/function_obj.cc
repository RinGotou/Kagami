#include "machine.h"

namespace kagami {
  Message FunctionGetId(ObjectMap &p) {
    auto &impl = p.Cast<FunctionImpl>(kStrMe);
    return Message(impl.GetId());
  }

  Message FunctionGetParameters(ObjectMap &p) {
    auto &impl = p.Cast<FunctionImpl>(kStrMe);
    shared_ptr<ObjectArray> dest_base = make_shared<ObjectArray>();
    auto origin_vector = impl.GetParameters();

    for (auto it = origin_vector.begin(); it != origin_vector.end(); ++it) {
      dest_base->emplace_back(Object(*it, kTypeIdString));
    }

    return Message().SetObject(Object(dest_base, kTypeIdArray));
  }

  Message FunctionCompare(ObjectMap &p) {
    auto &rhs = p[kStrRightHandSide];
    auto &lhs = p[kStrMe].Cast<FunctionImpl>();

    string type_id = rhs.GetTypeId();
    bool result = false;

    if (type_id == kTypeIdFunction) {
      auto &rhs_interface = rhs.Cast<FunctionImpl>();

      result = (lhs == rhs_interface);
    }

    return Message().SetObject(result);
  }

  void InitFunctionType() {
    using namespace management::type;

    ObjectTraitsSetup(kTypeIdFunction, PlainDeliveryImpl<FunctionImpl>)
      .InitComparator(PlainComparator<FunctionImpl>)
      .InitMethods(
        {
          FunctionImpl(FunctionGetId, "", "id"),
          FunctionImpl(FunctionGetParameters, "", "params"),
          FunctionImpl(FunctionCompare, kStrRightHandSide, kStrCompare)
        }
    );

    EXPORT_CONSTANT(kTypeIdFunction);
  }
}