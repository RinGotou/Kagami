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

  Object &Object::operator=(const Object &object) {
    ptr_ = object.ptr_;
    type_id_ = object.type_id_;
    ref_ = object.ref_;
    constructor_ = object.constructor_;
    return *this;
  }

  Object &Object::ManageContent(shared_ptr<void> ptr, string type_id) {
    if (ref_) return GetTargetObject()->ManageContent(ptr, type_id);
    ptr_ = ptr;
    type_id_ = type_id;
    return *this;
  }

  Object &Object::swap(Object &obj) {
    ptr_.swap(obj.ptr_);
    std::swap(type_id_, obj.type_id_);
    std::swap(ref_, obj.ref_);
    std::swap(constructor_, obj.constructor_);
    return *this;
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
    target.ptr->ref_count_ += 1;
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
    auto it = base_.find(id);

    it != base_.end() ? ptr = &(it->second) : nullptr;

    return ptr;
  }

  void ObjectContainer::ClearExcept(string exceptions) {
    map<string, Object> dest;
    map<string, Object>::iterator it;
    auto obj_list = BuildStringVector(exceptions);

    for (auto &unit : obj_list) {
      it = base_.find(unit);
      if (it != base_.end()) {
        dest.insert(*it);
      }
    }

    base_.swap(dest);
  }

  ObjectMap &ObjectMap::operator=(const initializer_list<NamedObject> &rhs) {
    this->clear();
    for (const auto &unit : rhs) {
      this->insert(unit);
    }

    return *this;
  }

  ObjectMap &ObjectMap::operator=(const ObjectMap &rhs) {
    this->clear();
    for (const auto &unit : rhs) {
      this->insert(unit);
    }
    return *this;
  }

  void ObjectMap::merge(ObjectMap &source) {
    for (auto &unit : source) {
      auto it = find(unit.first);
      if (it != end()) {
        it->second = unit.second;
      }
      else {
        this->insert(unit);
      }
    }
  }

  void ObjectStack::MergeMap(ObjectMap p) {
    if (p.empty()) return;

    auto &container = base_.back();
    for (auto &unit : p) {
      container.Add(unit.first, unit.second);
    }
  }

  Object *ObjectStack::Find(string id) {
    if (base_.empty() && prev_ == nullptr) return nullptr;
    ObjectPointer ptr = nullptr;
    
    for (auto it = base_.rbegin(); it != base_.rend(); ++it) {
      ptr = it->Find(id);
      if (ptr != nullptr) break;
    }

    if (prev_ != nullptr && ptr == nullptr) {
      ptr = prev_->Find(id);
    }

    return ptr;
  }

  bool ObjectStack::CreateObject(string id, Object obj) {
    if (base_.empty()) {
      if (prev_ == nullptr) {
        return false;
      }
      return prev_->CreateObject(id, obj);
    }
    auto &top = base_.back();

    if (top.Find(id) == nullptr) {
      top.Add(id, obj);
    }
    else {
      return false;
    }

    return true;
  }
}