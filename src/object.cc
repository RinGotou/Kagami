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

  shared_ptr<void> ObjectTraits::CreateObjectCopy(shared_ptr<void> target) const {
    shared_ptr<void> result = nullptr;
    if (target != nullptr) {
      result = dlvy_(target);
    }
    return result;
  }

  Object &Object::operator=(const Object &object) {
    if (object.mode_ == kObjectRef) {
      real_dest_ = object.real_dest_;
    }
    else {
      ptr_ = object.ptr_;
    }

    type_id_ = object.type_id_;
    mode_ = object.mode_;
    constructor_ = object.constructor_;
    return *this;
  }

  Object &Object::PackContent(shared_ptr<void> ptr, string type_id) {
    if (mode_ == kObjectRef) {
      return real_dest_->PackContent(ptr, type_id);
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
    std::swap(real_dest_, obj.real_dest_);
    return *this;
  }

  Object &Object::PackObject(Object &object) {
    type_id_ = object.type_id_;
    mode_ = kObjectRef;

    if (!object.IsRef()) {
      real_dest_ = &object;
    }
    else {
      real_dest_ = object.real_dest_;
    }

    real_dest_->ref_count_ += 1;
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

  bool ObjectContainer::Dispose(string id) {
    auto it = base_.find(id);
    bool result = it != base_.end();

    if (result) {
      base_.erase(it);
      BuildCache();
    }

    return result;
  }

  Object *ObjectContainer::Find(string id, bool forward_seeking) {
    if (base_.empty() && prev_ == nullptr) return nullptr;

    ObjectPointer ptr = nullptr;

    if (!base_.empty()) {
      auto it = dest_map_.find(id);

      if (it != dest_map_.end()) {
        ptr = it->second;
      }
      else {
        if (prev_ != nullptr && forward_seeking) {
          ptr = prev_->Find(id);
        }
      }
    }
    else if (prev_ != nullptr && forward_seeking) {
      ptr = prev_->Find(id);
    }

    return ptr;
  }

  string ObjectContainer::FindDomain(string id, bool forward_seeking) {
    if (base_.empty() && prev_ == nullptr) return kTypeIdNull;

    string result;

    if (!base_.empty()) {
      auto it = dest_map_.find(id);

      if (it != dest_map_.end()) {
        result = it->second->GetTypeId();
      }
      else {
        if (prev_ != nullptr && forward_seeking) {
          result = prev_->FindDomain(id);
        }
      }
    }
    else if (prev_ != nullptr && forward_seeking) {
      result = prev_->FindDomain(id); 
    }

    return result;
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

  void ObjectStack::MergeMap(ObjectMap &p) {
    if (p.empty()) return;

    auto &container = base_.back();
    for (auto &unit : p) {
      container.Add(unit.first, unit.second.IsRef() ?
        Object().PackObject(unit.second) :
        unit.second);
    }
  }

  Object *ObjectStack::Find(string id) {
    if (base_.empty() && prev_ == nullptr) return nullptr;
    ObjectPointer ptr = base_.back().Find(id);

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

    if (top.Find(id, false) == nullptr) {
      top.Add(id, obj);
    }
    else {
      return false;
    }

    return true;
  }

  bool ObjectStack::DisposeObjectInCurrentScope(string id) {
    if (base_.empty()) return false;
    auto &scope = base_.back();
    return scope.Dispose(id);
  }
}