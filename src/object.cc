#include "object.h"

namespace kagami {

  vector<string> BuildStringVector(string source) {
    vector<string> result;
    string temp;
    for (auto unit : source) {
      if (unit == '|') {
        result.push_back(temp);
        temp.clear();
        continue;
      }
      temp.append(1, unit);
    }
    if (temp != "") result.push_back(temp);
    return result;
  }

  Object &Object::CreateRef(Object &object) {
    type_id_ = object.type_id_;
    ref_ = true;

    TargetObject target;
    if (!object.IsRef()) {
      target.ptr = &object;
    }
    else {
      target.ptr = 
        static_pointer_cast<TargetObject>(object.ptr_)->ptr;
    }
    ptr_ = make_shared<TargetObject>(target);
    return *this;
  }

  Object &Object::CloneFrom(Object &object, bool force) {
    auto mod = [&]() {
      ptr_ = object.ptr_;
      type_id_ = object.type_id_;
      ref_ = object.ref_;
      constructor_ = object.constructor_;
    };

    if (force) {
      mod();
    }
    else {
      if (ref_) GetTargetObject()->CloneFrom(object);
      else mod();
    }
    return *this;
  }

  bool ObjectContainer::Add(string id, Object source) {
    if (CheckObject(id)) return false;
    base_.insert(NamedObject(id, source));
    return true;
  }

  Object *ObjectContainer::Find(string id) {
    if (base_.empty()) return nullptr;

    ObjectPointer ptr = nullptr;

    for (auto &unit : base_) {
      if (unit.first == id) {
        ptr = &(unit.second);
        break;
      }
    }

    return ptr;
  }

  void ObjectContainer::Dispose(string id) {
    auto it = base_.find(id);
    if (it != base_.end()) base_.erase(it);
  }
}