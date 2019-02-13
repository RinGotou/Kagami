#include "base_container.h"

namespace kagami {
  Message IteratorStepForward(ObjectMap &p) {
    auto &it = p[kStrObject].Cast<IteratorPackage>();
    it.StepForward();
    return Message();
  }

  Message IteratorForward(ObjectMap &p) {
    auto &obj = p[kStrObject];
    Object ret_obj(
      make_shared<IteratorPackage>(obj.Cast<IteratorPackage>().CreateCopy()),
      kTypeIdIterator
    );

    ret_obj.Cast<IteratorPackage>().StepForward(1);
    return Message().SetObject(ret_obj);
  }

  Message IteratorBack(ObjectMap &p) {
    auto &obj = p[kStrObject];
    Object ret_obj(
      make_shared<IteratorPackage>(obj.Cast<IteratorPackage>().CreateCopy()),
      kTypeIdIterator
    );

    ret_obj.Cast<IteratorPackage>().StepBack(1);
    return Message().SetObject(ret_obj);
  }

  Message IteratorStepBack(ObjectMap &p) {
    auto &it = p[kStrObject].Cast<IteratorPackage>();
    it.StepBack();
    return Message();
  }

  Message IteratorOperatorCompare(ObjectMap &p) {
    EXPECT_TYPE(p, kStrRightHandSide, kTypeIdIterator);
    auto &rhs = p[kStrRightHandSide].Cast<IteratorPackage>();
    auto &lhs = p[kStrObject].Cast<IteratorPackage>();
    return Message(util::MakeBoolean(lhs.Compare(rhs)));
  }

  Message IteratorGet(ObjectMap &p) {
    auto &it = p[kStrObject].Cast<IteratorPackage>();
    return Message().SetObject(Object().CreateRef(it.Deref()));
  }

  Message ArrayConstructor(ObjectMap &p) {
    shared_ptr<ObjectArray> base(make_shared<ObjectArray>());

    if (!p["size"].Null()) {
      size_t size = stol(p.Cast<string>("size"));
      EXPECT(size > 0, "Illegal array size.");

      Object obj;
      obj.CloneFrom(p["init_value"]);

      base->reserve(size);
      auto type_id = obj.GetTypeId();

      for (auto count = 0; count < size; count++) {
        base->emplace_back(Object(management::type::GetObjectCopy(obj), type_id));
      }
    }

    return Message().SetObject(Object(base, kTypeIdArray).SetConstructorFlag());
  }

  Message ArrayGetElement(ObjectMap &p) {
    EXPECT_TYPE(p, "index", kTypeIdRawString);

    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);
    //DEBUG_EVENT("(ArrayGetElement Interface)Index:" + p.Cast<string>("index"));
    size_t idx = stol(p.Cast<string>("index"));
    size_t size = base.size();

    EXPECT(idx < size, "Subscript is out of range. - " + to_string(idx));

    return Message().SetObject(Object().CreateRef(base[idx]));
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrObject];
    return Message(to_string(obj.Cast<ObjectArray>().size()));
  }

  Message ArrayEmpty(ObjectMap &p) {
    return Message(
      util::MakeBoolean(p[kStrObject].Cast<ObjectArray>().empty())
    );
  }

  Message ArrayPush(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);

    base.emplace_back(p["object"]);

    return Message();
  }

  Message ArrayPop(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrObject);

    if (!base.empty()) base.pop_back();

    return Message().SetObject(util::MakeBoolean(base.empty()));
  }

  Message ArrayPrint(ObjectMap &p) {
    Message result;
    ObjectMap obj_map;

    auto &base = p.Cast<ObjectArray>(kStrObject);
    auto interface = management::Order("print", kTypeIdNull, -1);

    for (auto &unit : base) {
      obj_map.insert(NamedObject(kStrObject, unit));
      result = interface.Start(obj_map);
      obj_map.clear();
    }

    return result;
  }

  Message ArrayBegin(ObjectMap &p) {
    auto &base = p[kStrObject].Cast<ObjectArray>();
    shared_ptr<IteratorPackage> pkg(
      new IteratorPackage(base.begin(), kContainerObjectArray)
    );
    return Message().SetObject(
      Object(pkg, kTypeIdIterator)
    );
  }

  Message ArrayEnd(ObjectMap &p) {
    auto &base = p[kStrObject].Cast<ObjectArray>();
    shared_ptr<IteratorPackage> pkg(
      new IteratorPackage(base.end(), kContainerObjectArray)
    );
    return Message().SetObject(
      Object(pkg, kTypeIdIterator)
    );
  }

  void InitContainerComponents() {
    using management::type::NewTypeSetup;

    NewTypeSetup(kTypeIdArray, [](shared_ptr<void> source) -> shared_ptr<void> {
      auto &src_base = *static_pointer_cast<ObjectArray>(source);
      shared_ptr<ObjectArray> dest_base = make_shared<ObjectArray>();

      dest_base->reserve(src_base.size());

      for (auto &unit : src_base) {
        dest_base->emplace_back(Object(management::type::GetObjectCopy(unit), unit.GetTypeId()));
      }

      return dest_base;
    })
      .InitConstructor(
        Interface(ArrayConstructor, "size|init_value", "array", kCodeAutoFill)
      )
      .InitMethods(
        {
          Interface(ArrayGetElement, "index", "__at"),
          Interface(ArrayPrint, "", "__print"),
          Interface(ArrayGetSize, "", "size"),
          Interface(ArrayPush, "object", "push"),
          Interface(ArrayPop, "object", "pop"),
          Interface(ArrayEmpty, "", "empty"),
          Interface(ArrayBegin, "", "head"),
          Interface(ArrayEnd, "", "tail"),
        }
    );

    NewTypeSetup(kTypeIdIterator, SimpleSharedPtrCopy<IteratorPackage>)
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
  }
}