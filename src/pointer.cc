#include "machine.h"

namespace kagami {
  void InitExternalPointerComponents() {
    using namespace mgmt::type;

    ObjectTraitsSetup(kTypeIdFunctionPointer, PlainDeliveryImpl<CABIContainer>, PlainHasher<CABIContainer>)
      .InitComparator(PlainComparator<CABIContainer>);

    ObjectTraitsSetup(kTypeIdObjectPointer, PlainDeliveryImpl<GenericPointer>, PlainHasher<GenericPointer>)
      .InitComparator(PlainComparator<GenericPointer>);

    EXPORT_CONSTANT(kTypeIdFunctionPointer);
    EXPORT_CONSTANT(kTypeIdObjectPointer);
  }
}