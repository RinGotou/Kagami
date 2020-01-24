#include "machine.h"

namespace kagami {
  Message NewExtension(ObjectMap &p) { 
    using namespace ext;
    auto tc = TypeChecking({ Expect("path",kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);
    auto &path = p.Cast<string>("path");
    ManagedExtension extension = make_shared<Extension>(path);
    auto loader = extension->GetExtensionLoader();

    if (loader == nullptr) {
      return Message("Invalid extension interface", kStateError);
    }

    ExtInterfaces interfaces{
      ext::GetCallbackFacilities,
      DisposeMemoryUnit,
      DisposeMemoryUnitGroup,
      FetchObjectType,
      ReceiveError
    };

    auto result = loader(&interfaces);

    if (result != 1) {
      return Message("Error is occurred while core is trying to load extension");
    }

    return Message().SetObject(Object(extension, kTypeIdExtension));
  }

  Message ExtensionGood(ObjectMap &p) {
    auto &extension = p.Cast<Extension>(kStrMe);
    return Message().SetObject(extension.Good());
  }

  Message ExtensionFetchFunction(ObjectMap &p) {
    auto tc = TypeChecking({ Expect("id",kTypeIdString) }, p);
    if (TC_FAIL(tc)) return TC_ERROR(tc);
    auto &extension = p.Cast<Extension>(kStrMe);
    auto &id = p.Cast<string>("id");
    auto activity = extension.FetchFunction(id);
    auto informer = extension.GetParameterInformer();

    if (activity != nullptr && informer != nullptr) {
      string param_pattern(informer(id.data()));
      shared_ptr<FunctionImpl> impl_ptr = 
        make_shared<FunctionImpl>(activity, id, param_pattern);
      return Message().SetObject(Object(impl_ptr, kTypeIdFunction));
    }

    return Message().SetObject(Object());
  }

  void InitExtensionComponents() {
    using namespace mgmt::type;

    ObjectTraitsSetup(kTypeIdExtension, ShallowDelivery)
      .InitConstructor(
        FunctionImpl(NewExtension, "path", kTypeIdExtension)
      )
      .InitMethods(
        {
          FunctionImpl(ExtensionGood, "", "good"),
          FunctionImpl(ExtensionFetchFunction, "id", "fetch")
        }
    );

    EXPORT_CONSTANT(kTypeIdExtension);
  }
}