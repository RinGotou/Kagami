#pragma once
#include "common.h"
#include "list.h" 

namespace kagami {
  class Object;
  class ObjectPlanner;
  class ObjectContainer;
  class ObjectMap;

  using ObjectPointer = Object * ;
  using ObjectPair = pair<string, Object>;
  using Activity = Message(*)(ObjectMap &);
  using NamedObject = pair<string, Object>;
  using ObjTypeId = string;

  using ContainerPool = kagami::list<ObjectContainer>;

  /*Object Class
  A shared void pointer is packaged in this.Almost all variables and
  constants are managed by shared pointers.This class will be packaged
  in ObjectContainer class.
  */
  class Object {
    using TargetObject = struct { Object *ptr; };
    std::shared_ptr<void> ptr_;
    string type_id_;
    string methods_;
    TokenTypeEnum token_type_;
    bool ro_, ref_, constructor_;

    Object *GetTargetObject() { 
      return static_pointer_cast<TargetObject>(ptr_)->ptr; 
    }
  public:
    Object();
    Object &Manage(string t, TokenTypeEnum tokenType);
    Object &Set(shared_ptr<void> ptr, string type_id);
    Object &Set(shared_ptr<void> ptr, string type_id, string methods, bool ro);
    Object &Ref(Object &object);
    void Clear();
    bool Compare(Object &object) const;
    Object &Copy(Object &object, bool force = false);
    shared_ptr<void> Get();
    string GetTypeId();
    Object &SetMethods(string methods);
    Object &AppendMethod(string method);
    Object &SetTokenType(TokenTypeEnum token_type);
    Object &SetRo(bool ro);
    string GetMethods();
    TokenTypeEnum GetTokenType();
    bool IsRo();
    bool ConstructorFlag();

    Object &SetConstructorFlag() { 
      constructor_ = true; 
      return *this; 
    }

    Object &Copy(Object &&object) { 
      return this->Copy(object); 
    }

    bool IsRef() const { 
      return ref_; 
    }

    bool operator==(Object &object) const { 
      return Compare(object); 
    }
    bool operator!=(Object &object) const { 
      return !Compare(object); 
    }
  };

  /* Object Template Class
  this class contains custom class info for script language.
  */
  class ObjectPlanner {
  private:
    CopyCreator creator_;
    string methods_;
  public:
    ObjectPlanner() : 
      methods_(kStrEmpty) { 

      creator_ = nullptr; 
    }
    ObjectPlanner(CopyCreator creator, 
      string methods) : 
      methods_(methods){

      creator_ = creator;
    }

    shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) const {
      shared_ptr<void> result = nullptr;
      if (target != nullptr) {
        result = creator_(target);
      }
      return result;
    }

    string GetMethods() const { 
      return methods_; 
    }
  };

  /*ObjectContainer Class
  MemoryManger will be filled with Object and manage life cycle of variables
  and constants.
  */
  class ObjectContainer {
  private:
    list<NamedObject> base;

    bool CheckObject(string sign) {
      for (size_t i = 0; i < base.size(); ++i) {
        NamedObject &object = base.at(i);
        if (object.first == sign) return false;
      }
      return true;
    }
  public:
    bool Add(string sign, Object &source);
    Object *Find(string sign);
    void Dispose(string sign);
    void clear();

    ObjectContainer() {}

    ObjectContainer(ObjectContainer &&mgr) {}

    ObjectContainer(ObjectContainer &container) {
      this->base.copy(container.base);
    }

    bool Empty() const { 
      return base.empty(); 
    }

    ObjectContainer &operator=(ObjectContainer &mgr) { 
      base.copy(mgr.base); return *this; 
    }
  };

  class ObjectMap : public map<string, Object> {
  protected:
    using ComparingFunction = bool(*)(Object &);
  public:
    bool Search(string id) {
      auto it = this->find(id);
      return it != this->end();
    }

    template <class T>
    T &Get(string id) {
      return *static_pointer_cast<T>(this->at(id).Get());
    }

    Object &operator()(string id) {
      return this->at(id);
    }

    bool CheckTypeId(string id, string type_id) {
      return this->at(id).GetTypeId() == type_id;
    }

    bool CheckTypeId(string id, ComparingFunction func) {
      return func(this->at(id));
    }

    void Input(string id, Object &obj) {
      this->insert(NamedObject(id, obj));
    }

    void Input(string id) {
      this->insert(NamedObject(id, Object()));
    }
  };
}
