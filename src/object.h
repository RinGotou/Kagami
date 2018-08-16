#pragma once
#include "common.h"
#include "list.h" 

namespace kagami {
  /*Object Class
  A shared void pointer is packaged in this.Almost all variables and
  constants are managed by shared pointers.This class will be packaged
  in ObjectManager class.
  */
  class Object {
    using TargetObject = struct { Object *ptr; };
    std::shared_ptr<void> ptr;
    string typeId;
    string methods;
    TokenTypeEnum tokenTypeEnum;
    bool ro, permanent, ref;

    Object *GetTargetObject() { return static_pointer_cast<TargetObject>(ptr)->ptr; }
  public:
    Object();
    Object &Manage(string t, string typeId = kTypeIdRawString);
    Object &Set(shared_ptr<void> ptr, string typeId);
    Object &Ref(Object &object);
    void Clear();
    bool Compare(Object &object) const;
    Object &Copy(Object &object);
    shared_ptr<void> Get();
    string GetTypeId();
    Object &SetMethods(string methods);
    Object &SetTokenType(TokenTypeEnum tokenTypeEnum);
    Object &SetRo(bool ro);
    Object &SetPermanent(bool permanent);
    string GetMethods();
    TokenTypeEnum GetTokenType();
    bool IsRo();

    bool IsPermanent() const { return permanent; }
    Object &Copy(Object &&object) { return this->Copy(object); }
    bool IsRef() const { return ref; }
    bool operator==(Object &object) const { return Compare(object); }
    bool operator!=(Object &object) const { return !Compare(object); }

  };

  /* Object Template Class
  this class contains custom class info for script language.
  */
  class ObjectPlanner {
  private:
    CopyCreator copyCreator;
    string methods;
  public:
    ObjectPlanner() : methods(kStrEmpty) { copyCreator = nullptr; }
    ObjectPlanner(CopyCreator copyCreator, string methods) : methods(methods){
      this->copyCreator = copyCreator;
    }

    shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) const {
      shared_ptr<void> result = nullptr;
      if (target != nullptr) {
        result = copyCreator(target);
      }
      return result;
    }

    string GetMethods() const { return methods; }
  };

  /*ObjectManager Class
  MemoryManger will be filled with Object and manage life cycle of variables
  and constants.
  */
  class ObjectManager {
  public:
    ObjectManager() {}
    ObjectManager(ObjectManager &&mgr) {}

    bool Add(string sign, Object source);
    Object *Find(string sign);
    void Dispose(string sign);
    void clear();

    bool Empty() const { return base.empty(); }
    ObjectManager(ObjectManager &mgr) { base = mgr.base; }
    ObjectManager &operator=(ObjectManager &mgr) { base = mgr.base; return *this; }
  private:
    list<NamedObject> base;

    bool CheckObject(string sign) {
      for (size_t i = 0; i < base.size(); ++i) {
        NamedObject &object = base.at(i);
        if (object.first == sign) return false;
      }
      return true;
    }
  };
}
