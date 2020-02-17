#include "machine.h"

namespace kagami {


  void InitStructComponents() {
    using namespace mgmt;
    using namespace mgmt::type;

    ObjectTraitsSetup(kTypeIdStruct, ShallowDelivery);
  }
}