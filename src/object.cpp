#include "object.h"

namespace kagami {

  Object &Object::Ref(Object &object) {
    type_id_ = kTypeIdRef;
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

  Object &Object::Copy(Object &object, bool force) {
    auto mod = [&]() {
      ptr_ = object.ptr_;
      type_id_ = object.type_id_;
      methods_ = object.methods_;
      token_type_ = object.token_type_;
      ro_ = object.ro_;
      ref_ = object.ref_;
      constructor_ = object.constructor_;
    };

    if (force) {
      mod();
    }
    else {
      if (ref_) GetTargetObject()->Copy(object);
      else mod();
    }
    return *this;
  }

  bool ObjectContainer::Add(string id, Object &source) {
    if (!CheckObject(id)) return false;
    base.push_back(NamedObject(id, Object().Copy(source)));
    return true;
  }

  Object *ObjectContainer::Find(string id, string domain) {
    Object *object = nullptr;
    if (base.empty()) return object;
    bool has_domain = (domain != kStrEmpty);

    for (size_t i = 0; i < base.size(); ++i) {
      NamedObject &obj = base[i];
      if (obj.first == id) {
        if (!has_domain || (has_domain && obj.second.GetDomain() == domain)) {
          object = &(base[i].second);
          break;
        }
      }
    }
    return object;
  }

  void ObjectContainer::Dispose(string id, string domain) {
    size_t pos = 0;
    bool found = false;
    bool has_domain = (domain != kStrEmpty);

    for (size_t i = 0; i < base.size(); ++i) {
      NamedObject &obj = base[i];
      if (obj.first == id) {
        if (!has_domain || (has_domain && obj.second.GetDomain() == domain)) {
          found = true;
          pos = i;
          break;
        }
      }
    }

    if (found) base.erase(pos);
  }
}