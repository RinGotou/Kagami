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
    if (object.mode_ == kObjectRef) {
      MakeRef(object);
    }
    else {
      ptr_ = object.ptr_;
    }

    type_id_ = object.type_id_;
    mode_ = object.mode_;
    constructor_ = object.constructor_;
    return *this;
  }

  Object &Object::Manage(shared_ptr<void> ptr, string type_id) {
    if (mode_ == kObjectRef) {
      return GetTargetObject()->Manage(ptr, type_id);
    }

    ptr_ = ptr;
    type_id_ = type_id;
    return *this;
  }

  Object &Object::swap(Object &obj) {
    ptr_.swap(obj.ptr_);
    std::swap(type_id_, obj.type_id_);
    std::swap(mode_, obj.mode_);
    std::swap(constructor_, obj.constructor_);
    return *this;
  }

  Object &Object::CreateRef(Object &object) {
    type_id_ = object.type_id_;
    mode_ = kObjectRef;

    if (!object.IsRef()) {
      TargetObject target;
      target.ptr = &object;
      ptr_ = make_shared<TargetObject>(target);
    }
    else {
      ptr_ = object.ptr_;
      //target.ptr = static_pointer_cast<TargetObject>(object.ptr_)->ptr;
    }
    static_pointer_cast<TargetObject>(ptr_)->ptr->ref_count_ += 1;
    //target.ptr->ref_count_ += 1;
    //ptr_ = make_shared<TargetObject>(target);
    return *this;
  }

  void ObjectContainer::BuildCache() {
    dest_map_.clear();
    const auto begin = base_.begin(), end = base_.end();
    for (auto it = begin; it != end; ++it) {
      dest_map_.insert(make_pair(it->first, &it->second));
    }
  }

  bool ObjectContainer::Add(string id, Object source) {
    if (CheckObject(id)) return false;
    base_.insert(NamedObject(id, source));
    BuildCache();
    return true;
  }

  Object *ObjectContainer::Find(string id) {
    if (base_.empty()) return nullptr;

    ObjectPointer ptr = nullptr;

    auto it = dest_map_.find(id);
    ptr = it != dest_map_.end() ? it->second : nullptr;
    //auto it = base_.find(id);
    //it != base_.end() ? ptr = &(it->second) : nullptr;

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
    BuildCache();
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
    const auto begin = base_.rbegin(), end = base_.rend();
    for (auto it = begin; it != end; ++it) {
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