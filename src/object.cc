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

  string CombineStringVector(vector<string> target) {
    string result;
    for (size_t i = 0; i < target.size(); ++i) {
      result = result + target[i] + "|";
    }
    result.pop_back();
    return result;
  }

  size_t PointerHasher(shared_ptr<void> ptr) {
    auto hasher = std::hash<shared_ptr<void>>();
    return hasher(ptr);
  }

  shared_ptr<void> ShallowDelivery(shared_ptr<void> target) {
    return target;
  }

  Object &Object::operator=(const Object &object) {
    if (object.mode_ == kObjectRef) {
      real_dest_ = object.real_dest_;
      ptr_.reset();
    }
    else {
      real_dest_ = nullptr;
      ptr_ = object.ptr_;
    }

    type_id_ = object.type_id_;
    mode_ = object.mode_;
    delivering_ = object.delivering_;
    return *this;
  }

  Object &Object::PackContent(shared_ptr<void> ptr, string type_id) {
    if (mode_ == kObjectRef) {
      return static_cast<ObjectPointer>(real_dest_)
        ->PackContent(ptr, type_id);
    }

    ptr_ = ptr;
    type_id_ = type_id;
    return *this;
  }

  Object &Object::swap(Object &obj) {
    ptr_.swap(obj.ptr_);
    std::swap(type_id_, obj.type_id_);
    std::swap(mode_, obj.mode_);
    std::swap(delivering_, obj.delivering_);
    std::swap(real_dest_, obj.real_dest_);
    return *this;
  }

  Object &Object::PackObject(Object &object) {
    ptr_.reset();
    type_id_ = object.type_id_;
    mode_ = kObjectRef;

    if (!object.IsRef()) {
      real_dest_ = &object;
    }
    else {
      real_dest_ = object.real_dest_;
    }

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
    if (IsDelegated()) return delegator_->Add(id, source);

    if (CheckObject(id)) return false;
    auto result = base_.insert(NamedObject(id, source));
    if (result.second) {
      dest_map_.insert(make_pair(id, &result.first->second));
    }

    return true;
  }

  bool ObjectContainer::Dispose(string id) {
    if (IsDelegated()) return delegator_->Dispose(id);

    auto it = base_.find(id);
    bool result = it != base_.end();

    if (result) {
      base_.erase(it);
      dest_map_.erase(id);
    }

    return result;
  }

  Object *ObjectContainer::Find(string id, bool forward_seeking) {
    if (IsDelegated()) return delegator_->Find(id, forward_seeking);

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

  Object *ObjectContainer::FindWithDomain(string id, string domain,
    bool forward_seeking) {
    if (IsDelegated()) return delegator_->FindWithDomain(id, domain, forward_seeking);
  
    if (base_.empty() && prev_ == nullptr) return nullptr;

    ObjectPointer container_ptr = nullptr;

    //Find sub-container
    if (!base_.empty()) {
      auto it = dest_map_.find(domain);

      if (it != dest_map_.end()) {
        container_ptr = it->second;
      }
      else {
        if (prev_ != nullptr && forward_seeking) {
          container_ptr = prev_->Find(id);
        }
      }
    }
    else if (prev_ != nullptr && forward_seeking) {
      container_ptr = prev_->Find(id);
    }

    if (container_ptr == nullptr) return nullptr;
    if (!container_ptr->IsSubContainer()) return nullptr;

    auto &sub_container = container_ptr->Cast<ObjectStruct>();
    return sub_container.Find(id, false);
  }

  bool ObjectContainer::IsInside(Object *ptr) {
    if (IsDelegated()) return delegator_->IsInside(ptr);

    bool result = false;
    for (const auto &unit : dest_map_) {
      if (unit.second == ptr) result = true;
    }
    return result;
  }

  void ObjectContainer::ClearExcept(string exceptions) {
    if (IsDelegated()) delegator_->ClearExcept(exceptions);

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

  void ObjectMap::Naturalize(ObjectContainer &container) {
    for (auto it = begin(); it != end(); ++it) {
      if (it->second.IsRef() && container.IsInside(it->second.GetRealDest())) {
        it->second = it->second.Unpack();
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

  Object *ObjectStack::Find(string id, string domain) {
    if (base_.empty() && prev_ == nullptr) return nullptr;
    ObjectPointer ptr = base_.back().FindWithDomain(id, domain);

    if (prev_ != nullptr && ptr == nullptr) {
      ptr = prev_->Find(id, domain);
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

  bool ObjectStack::DisposeObject(string id) {
    if (base_.empty()) return false;
    bool result = false;

    for (auto it = base_.rbegin(); it != base_.rend(); it++) {
      result = it->Dispose(id);

      if (result) break;
    }

    return result;
  }
}