#include "containers.h"

namespace kagami {
  Message IteratorStepForward(ObjectMap &p) {
    auto &it = p[kStrObject].Cast<UnifiedIterator>();
    it.StepForward();
    return Message();
  }

  Message IteratorForward(ObjectMap &p) {
    auto &obj = p[kStrObject];
    Object ret_obj(
      make_shared<UnifiedIterator>(obj.Cast<UnifiedIterator>().CreateCopy()),
      kTypeIdIterator
    );

    ret_obj.Cast<UnifiedIterator>().StepForward(1);
    return Message().SetObject(ret_obj);
  }

  Message IteratorBack(ObjectMap &p) {
    auto &obj = p[kStrObject];
    Object ret_obj(
      make_shared<UnifiedIterator>(obj.Cast<UnifiedIterator>().CreateCopy()),
      kTypeIdIterator
    );

    ret_obj.Cast<UnifiedIterator>().StepBack(1);
    return Message().SetObject(ret_obj);
  }

  Message IteratorStepBack(ObjectMap &p) {
    auto &it = p[kStrObject].Cast<UnifiedIterator>();
    it.StepBack();
    return Message();
  }

  Message IteratorOperatorCompare(ObjectMap &p) {
    EXPECT_TYPE(p, kStrRightHandSide, kTypeIdIterator);
    auto &rhs = p[kStrRightHandSide].Cast<UnifiedIterator>();
    auto &lhs = p[kStrObject].Cast<UnifiedIterator>();
    return Message().SetObject(lhs.Compare(rhs));
  }

  Message IteratorGet(ObjectMap &p) {
    auto &it = p[kStrObject].Cast<UnifiedIterator>();
    return Message().SetObject(Object().CreateRef(it.Deref()));
  }

  Message NewArray(ObjectMap &p) {
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

    if (!p["size"].Null()) {
      size_t size = p.Cast<int64_t>("size");
      EXPECT(size > 0, "Illegal array size.");

      Object obj = p["init_value"];

      base->reserve(size);
      auto type_id = obj.GetTypeId();

      for (size_t count = 0; count < size; count++) {
        base->emplace_back(Object(management::type::GetObjectCopy(obj), type_id));
      }
    }

    return Message().SetObject(Object(base, kTypeIdArray).SetConstructorFlag());
  }

  Message ArrayGetElement(ObjectMap &p) {
    EXPECT_TYPE(p, "index", kTypeIdInt);

    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);
    size_t idx = p.Cast<int64_t>("index");
    size_t size = base.size();

    EXPECT(idx < size, "Subscript is out of range. - " + to_string(idx));

    return Message().SetObject(Object().CreateRef(base[idx]));
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrObject];
    int64_t size = static_cast<int64_t>(obj.Cast<ObjectArray>().size());
    return Message().SetObject(Object(make_shared<int64_t>(size), kTypeIdInt));
  }

  Message ArrayEmpty(ObjectMap &p) {
    return Message().SetObject(p[kStrObject].Cast<ObjectArray>().empty());
  }

  Message ArrayPush(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);
    Object obj(management::type::GetObjectCopy(p["object"]), p["object"].GetTypeId());
    base.emplace_back(obj);

    return Message();
  }

  Message ArrayPop(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);
    if (!base.empty()) base.pop_back();

    return Message().SetObject(base.empty());
  }

  Message ArrayBegin(ObjectMap &p) {
    auto &base = p[kStrObject].Cast<ObjectArray>();
    shared_ptr<UnifiedIterator> pkg(
      new UnifiedIterator(base.begin(), kContainerObjectArray)
    );
    return Message().SetObject(
      Object(pkg, kTypeIdIterator)
    );
  }

  Message ArrayEnd(ObjectMap &p) {
    auto &base = p[kStrObject].Cast<ObjectArray>();
    shared_ptr<UnifiedIterator> pkg(
      new UnifiedIterator(base.end(), kContainerObjectArray)
    );
    return Message().SetObject(
      Object(pkg, kTypeIdIterator)
    );
  }

  Message ArrayClear(ObjectMap &p) {
    auto &base = p.Cast<ObjectArray>(kStrObject);
    base.clear();
    base.shrink_to_fit();
    return Message();
  }

  size_t ArrayHasher(shared_ptr<void> ptr) {
    auto &base = *static_pointer_cast<ObjectArray>(ptr);
    size_t result = 0;

    for (auto it = base.begin(); it != base.end(); ++it) {
      if (management::type::IsHashable(*it)) {
        result ^= management::type::GetHash(*it);
        result = result << 1;
      }
    }

    return result;
  }

  shared_ptr<void> ArrayCopyingPolicy(shared_ptr<void> ptr) {
    auto &src_base = *static_pointer_cast<ObjectArray>(ptr);
    shared_ptr<ObjectArray> dest_base = make_shared<ObjectArray>();

    dest_base->reserve(src_base.size());

    for (auto &unit : src_base) {
      dest_base->emplace_back(Object(management::type::GetObjectCopy(unit), unit.GetTypeId()));
    }

    return dest_base;
  }

  void InitContainerComponents() {
    using management::type::NewTypeSetup;
    using management::type::CustomHasher;

    NewTypeSetup(kTypeIdArray, ArrayCopyingPolicy, CustomHasher<ObjectArray, ArrayHasher>())
      .InitConstructor(
        Interface(NewArray, "size|init_value", "array", kCodeAutoFill)
      )
      .InitMethods(
        {
          Interface(ArrayGetElement, "index", "__at"),
          Interface(ArrayGetSize, "", "size"),
          Interface(ArrayPush, "object", "push"),
          Interface(ArrayPop, "object", "pop"),
          Interface(ArrayEmpty, "", "empty"),
          Interface(ArrayBegin, "", "head"),
          Interface(ArrayEnd, "", "tail"),
          Interface(ArrayClear, "", "clear")
        }
    );

    NewTypeSetup(kTypeIdIterator, SimpleSharedPtrCopy<UnifiedIterator>)
      .InitMethods(
        {
          Interface(IteratorGet, "", "get"),
          Interface(IteratorForward, "", "forward"),
          Interface(IteratorBack, "", "back"),
          Interface(IteratorStepForward, "", "step_forward"),
          Interface(IteratorStepBack, "", "step_back"),
          Interface(IteratorOperatorCompare, kStrRightHandSide, kStrCompare)
        }
    );

    EXPORT_CONSTANT(kTypeIdArray);
    EXPORT_CONSTANT(kTypeIdIterator);
  }
}