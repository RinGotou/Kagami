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
    info_ = object.info_;

    if (info_.mode != kObjectRef) {
      info_.real_dest = nullptr;
      dynamic_cast<shared_ptr<void> *>(this)->operator=(object);
    }
    else {
      reset();
    }

    return *this;
  }

  Object &Object::PackContent(shared_ptr<void> ptr, string type_id) {
    if (info_.mode == kObjectRef) {
      return static_cast<ObjectPointer>(info_.real_dest)
        ->PackContent(ptr, type_id);
    }

    dynamic_cast<shared_ptr<void> *>(this)->operator=(ptr);
    info_.type_id = type_id;
    return *this;
  }

  Object &Object::swap(Object &obj) {
    dynamic_cast<shared_ptr<void> *>(this)->swap(obj);
    std::swap(info_.type_id, obj.info_.type_id);
    std::swap(info_.mode, obj.info_.mode);
    std::swap(info_.delivering, obj.info_.delivering);
    std::swap(info_.real_dest, obj.info_.real_dest);
    std::swap(info_.sub_container, obj.info_.sub_container);
    std::swap(info_.alive, obj.info_.alive);
    return *this;
  }

  Object &Object::PackObject(Object &object) {
    reset();
    info_.type_id = object.info_.type_id;
    info_.mode = kObjectRef;

    if (!object.IsRef()) {
      info_.real_dest = &object;
      info_.alive = true;
    }
    else {
      info_.real_dest = object.info_.real_dest;
      info_.alive = object.info_.alive;
    }

    if (object.IsAlive()) EstablishRefLink();

    return *this;
  }

  void ObjectContainer::BuildCache() {
    if (prev_ != nullptr) {
      if (prev_->dest_map_.empty()) dest_map_.clear();
      else dest_map_ = prev_->dest_map_;
    }

    dest_map_.rehash(dest_map_.size() + base_.size());
    const auto begin = base_.begin(), end = base_.end();
    for (auto it = begin; it != end; ++it) {
      dest_map_.insert_or_assign(it->first, &it->second);
    }
  }

  bool ObjectContainer::Add(string id, Object &source) {
    if (IsDelegated()) return delegator_->Add(id, source);

    if (CheckObject(id)) return false;
    auto result = base_.emplace(NamedObject(id, source));
    if (result.second) {
      dest_map_.insert_or_assign(id, &result.first->second);
      dest_map_.rehash(0);
    }

    return true;
  }

  bool ObjectContainer::Add(string id, Object &&source) {
    if (IsDelegated()) return delegator_->Add(id, std::move(source));

    if (CheckObject(id)) return false;
    auto result = base_.emplace(NamedObject(id, std::move(source)));
    if (result.second) {
      dest_map_.insert_or_assign(id, &result.first->second);
      dest_map_.rehash(0);
    }

    return true;
  }

  void ObjectContainer::Replace(string id, Object &source) {
    if (IsDelegated()) delegator_->Replace(id, source);

    base_[id] = source;
    dest_map_[id] = &base_[id];
  }

  void ObjectContainer::Replace(string id, Object &&source) {
    if (IsDelegated()) delegator_->Replace(id, std::move(source));

    base_[id] = source;
    dest_map_[id] = &base_[id];
  }

  Object *ObjectContainer::Find(const string &id, bool forward_seeking) {
    if (IsDelegated()) return delegator_->Find(id, forward_seeking);

    ObjectPointer ptr = nullptr;

    auto it = dest_map_.find(id);

    if (it != dest_map_.end()) {
      ptr = it->second;
    }

    return ptr;
  }

  Object *ObjectContainer::FindWithDomain(const string &id, 
    const string &domain, bool forward_seeking) {
    if (IsDelegated()) return delegator_->FindWithDomain(id, domain, forward_seeking);
  
    if (base_.empty() && prev_ == nullptr) return nullptr;

    ObjectPointer container_ptr = nullptr;

    auto it = dest_map_.find(domain);

    if (it != dest_map_.end()) {
      container_ptr = it->second;
    }

    if (container_ptr == nullptr) return nullptr;
    if (!container_ptr->IsSubContainer()) return nullptr;

    auto &sub_container = container_ptr->Cast<ObjectStruct>();
    return sub_container.Find(id, false);
  }

  bool ObjectContainer::IsInside(Object *ptr) {
    if (IsDelegated()) return delegator_->IsInside(ptr);

    bool result = false;
    for (const auto &unit : base_) {
      if (&unit.second == ptr) result = true;
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

  Object *ObjectStack::Find(const string &id) {
    if (base_.empty() && prev_ == nullptr) return nullptr;
    ObjectPointer ptr = base_.back().Find(id);

    if (prev_ != nullptr && ptr == nullptr) {
      ptr = prev_->Find(id);
    }

    return ptr;
  }

  Object *ObjectStack::Find(const string &id, const string &domain) {
    if (base_.empty() && prev_ == nullptr) return nullptr;
    ObjectPointer ptr = base_.back().FindWithDomain(id, domain);

    if (prev_ != nullptr && ptr == nullptr) {
      ptr = prev_->Find(id, domain);
    }

    return ptr;
  }

  bool ObjectStack::CreateObject(string id, Object &obj) {
    if (base_.empty()) {
      if (prev_ == nullptr) {
        return false;
      }
      return prev_->CreateObject(id, obj);
    }
    auto &top = base_.back();

    return top.Add(id, obj);
  }

  bool ObjectStack::CreateObject(string id, Object &&obj) {
    if (base_.empty()) {
      if (prev_ == nullptr) {
        return false;
      }
      return prev_->CreateObject(id, std::move(obj));
    }
    auto &top = base_.back();
    
    return top.Add(id, std::move(obj));
  }
}