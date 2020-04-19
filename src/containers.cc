#include "containers.h"

namespace kagami {
  Message IteratorStepForward(ObjectMap &p) {
    auto &it = p[kStrMe].Cast<UnifiedIterator>();
    it.StepForward();
    return Message();
  }

  Message IteratorStepBack(ObjectMap &p) {
    auto &it = p[kStrMe].Cast<UnifiedIterator>();
    it.StepBack();
    return Message();
  }

  Message IteratorOperatorCompare(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect(kStrRightHandSide, kTypeIdIterator) }, p
    );

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    auto &rhs = p[kStrRightHandSide].Cast<UnifiedIterator>();
    auto &lhs = p[kStrMe].Cast<UnifiedIterator>();
    return Message().SetObject(lhs.Compare(rhs));
  }

  Message IteratorGet(ObjectMap &p) {
    auto &it = p[kStrMe].Cast<UnifiedIterator>();
    return Message().SetObject(it.Unpack());
  }

  bool IteratorComparator(Object &lhs, Object &rhs) {
    auto &lhs_value = lhs.Cast<UnifiedIterator>();
    auto &rhs_value = rhs.Cast<UnifiedIterator>();
    return lhs_value.Compare(rhs_value);
  }

  Message NewArray(ObjectMap &p) {
    auto tc_result = TypeChecking(
      { Expect("size", kTypeIdInt) }, p,
      { "size" }
    );

    if (TC_FAIL(tc_result)) return TC_ERROR(tc_result);

    ManagedArray base = make_shared<ObjectArray>();

    if (!p["size"].Null()) {
      size_t size = p.Cast<int64_t>("size");
      if (size < 0) return Message("Invalid array size.", kStateError);

      Object obj = p["init_value"];

      auto type_id = obj.GetTypeId();

      for (size_t count = 0; count < size; ++count) {
        base->emplace_back(management::type::CreateObjectCopy(obj));
      }
    }

    return Message().SetObject(Object(base, kTypeIdArray));
  }

  Message ArrayGetElement(ObjectMap &p) {
    auto tc = TypeChecking(
      { Expect("index", kTypeIdInt) }, p
    );

    if (TC_FAIL(tc)) return TC_ERROR(tc);

    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    auto &idx = p.Cast<int64_t>("index");
    size_t size = base.size();

    if (size_t(idx) >= size) return Message("Subscript is out of range", kStateError);

    return Message().SetObjectRef(base[idx]);
  }

  Message ArrayGetSize(ObjectMap &p) {
    auto &obj = p[kStrMe];
    int64_t size = static_cast<int64_t>(obj.Cast<ObjectArray>().size());
    return Message().SetObject(Object(make_shared<int64_t>(size), kTypeIdInt));
  }

  Message ArrayEmpty(ObjectMap &p) {
    return Message().SetObject(p[kStrMe].Cast<ObjectArray>().empty());
  }

  Message ArrayPush(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    Object obj = management::type::CreateObjectCopy(p["object"]);
    base.emplace_back(obj);

    return Message();
  }

  Message ArrayPop(ObjectMap &p) {
    ObjectArray &base = p.Cast<ObjectArray>(kStrMe);
    if (!base.empty()) base.pop_back();

    return Message().SetObject(base.empty());
  }

  Message ArrayHead(ObjectMap &p) {
    auto &base = p[kStrMe].Cast<ObjectArray>();
    shared_ptr<UnifiedIterator> it = 
      make_shared<UnifiedIterator>(base.begin(), kContainerObjectArray);
    return Message().SetObject(Object(it, kTypeIdIterator));
  }

  Message ArrayTail(ObjectMap &p) {
    auto &base = p[kStrMe].Cast<ObjectArray>();
    shared_ptr<UnifiedIterator> it = 
      make_shared<UnifiedIterator>(base.end(), kContainerObjectArray);
    return Message().SetObject(Object(it, kTypeIdIterator));
  }

  Message ArrayClear(ObjectMap &p) {
    auto &base = p.Cast<ObjectArray>(kStrMe);
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

  shared_ptr<void> ArrayDelivery(shared_ptr<void> ptr) {
    using namespace management::type;
    auto &src_base = *static_pointer_cast<ObjectArray>(ptr);
    ManagedArray dest_base = make_shared<ObjectArray>();

    for (auto &unit : src_base) {
      dest_base->emplace_back(CreateObjectCopy(unit));
    }

    return dest_base;
  }

  Message NewPair(ObjectMap &p) {
    auto &left = p["left"];
    auto &right = p["right"];
    ManagedPair pair = make_shared<ObjectPair>(
      management::type::CreateObjectCopy(left),
      management::type::CreateObjectCopy(right));
    return Message().SetObject(Object(pair, kTypeIdPair));
  }

  Message PairLeft(ObjectMap &p) {
    auto &base = p.Cast<ObjectPair>(kStrMe);
    return Message().SetObject(Object().PackObject(base.first));
  }

  Message PairRight(ObjectMap &p) {
    auto &base = p.Cast<ObjectPair>(kStrMe);
    return Message().SetObject(Object().PackObject(base.second));
  }

  shared_ptr<void> PairDelivery(shared_ptr<void> ptr) {
    auto &src_base = *static_pointer_cast<ObjectPair>(ptr);
    ManagedPair dest_base = make_shared<ObjectPair>(
      management::type::CreateObjectCopy(src_base.first),
      management::type::CreateObjectCopy(src_base.second)
      );
    return dest_base;
  }

  Message NewTable(ObjectMap &p) {
    ManagedTable table = make_shared<ObjectTable>();
    return Message().SetObject(Object(table, kTypeIdTable));
  }

  Message TableInsert(ObjectMap &p) {
    using namespace management::type;
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto &value = p["value"];
    auto result = table.insert(
      make_pair(CreateObjectCopy(key), CreateObjectCopy(value))
    );
    return Message();
  }

  Message TableGetElement(ObjectMap &p) {
    // Ref: https://stackoverflow.com/questions/53149145/
    // Seems nothing to worry about pointers to element

    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &dest_key = p["key"];
    auto &result = table[dest_key];
    return Message().SetObjectRef(result);
    //return Message().SetObject(Object().PackObject(result));
  }

  Message TableFindElement(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto it = table.find(key);
    if (it != table.end()) {
      return Message().SetObject(Object().PackObject(it->second));
    }
    return Message().SetObject(Object());
  }

  Message TableEraseElement(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    auto &key = p["key"];
    auto count = table.erase(key);
    return Message().SetObject(static_cast<int64_t>(count));
  }

  Message TableEmpty(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    return Message().SetObject(table.empty());
  }

  Message TableSize(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    return Message().SetObject(static_cast<int64_t>(table.size()));
  }

  Message TableClear(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    table.clear();
    return Message();
  }

  Message TableHead(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    shared_ptr<UnifiedIterator> it =
      make_shared<UnifiedIterator>(table.begin(), kContainerObjectTable);
    return Message().SetObject(Object(it, kTypeIdIterator));
  }

  Message TableTail(ObjectMap &p) {
    auto &table = p.Cast<ObjectTable>(kStrMe);
    shared_ptr<UnifiedIterator> it =
      make_shared<UnifiedIterator>(table.end(), kContainerObjectTable);
    return Message().SetObject(Object(it, kTypeIdIterator));
  }

  shared_ptr<void> TableDelivery(shared_ptr<void> ptr) {
    using namespace management::type;
    auto &table = *static_pointer_cast<ObjectTable>(ptr);
    ManagedTable dest = make_shared<ObjectTable>();
    Object key_copy;

    for (auto &unit : table) {
      key_copy = unit.first;
      dest->insert(make_pair(CreateObjectCopy(key_copy), 
          CreateObjectCopy(unit.second)));
    }

    return dest;
  }

  void InitContainerComponents() {
    using management::type::ObjectTraitsSetup;

    //TODO:insert()
    ObjectTraitsSetup(kTypeIdArray, ArrayDelivery, ArrayHasher)
      .InitConstructor(
        FunctionImpl(NewArray, "size|init_value", "array", kParamAutoFill).SetLimit(0)
      )
      .InitMethods(
        {
          FunctionImpl(ArrayGetElement, "index", kStrAt),
          FunctionImpl(ArrayGetSize, "", "size"),
          FunctionImpl(ArrayPush, "object", "push"),
          FunctionImpl(ArrayPop, "", "pop"),
          FunctionImpl(ArrayEmpty, "", "empty"),
          FunctionImpl(ArrayHead, "", "head"),
          FunctionImpl(ArrayTail, "", "tail"),
          FunctionImpl(ArrayClear, "", "clear")
        }
    );

    ObjectTraitsSetup(kTypeIdIterator, PlainDeliveryImpl<UnifiedIterator>)
      .InitComparator(IteratorComparator)
      .InitMethods(
        {
          FunctionImpl(IteratorGet, "", "obj"),
          FunctionImpl(IteratorStepForward, "", "step_forward"),
          FunctionImpl(IteratorStepBack, "", "step_back"),
          FunctionImpl(IteratorOperatorCompare, kStrRightHandSide, kStrCompare)
        }
    );

    ObjectTraitsSetup(kTypeIdPair, PairDelivery)
      .InitConstructor(
        FunctionImpl(NewPair, "left|right", "pair")
      )
      .InitMethods(
        {
          FunctionImpl(PairLeft, "", "left"),
          FunctionImpl(PairRight, "", "right")
        }
    );

    ObjectTraitsSetup(kTypeIdTable, TableDelivery)
      .InitConstructor(
        FunctionImpl(NewTable, "", "table")
      )
      .InitMethods(
        {
          FunctionImpl(TableInsert, "key|value", "insert"),
          FunctionImpl(TableGetElement, "key", kStrAt),
          FunctionImpl(TableFindElement, "key", "find"),
          FunctionImpl(TableEraseElement, "key", "erase"),
          FunctionImpl(TableEmpty, "", "empty"),
          FunctionImpl(TableSize, "", "size"),
          FunctionImpl(TableClear, "", "clear"),
          FunctionImpl(TableHead, "", "head"),
          FunctionImpl(TableTail, "", "tail")
        }
    );

    EXPORT_CONSTANT(kTypeIdArray);
    EXPORT_CONSTANT(kTypeIdIterator);
    EXPORT_CONSTANT(kTypeIdPair);
    EXPORT_CONSTANT(kTypeIdTable);
  }
}