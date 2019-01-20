#pragma once
#include "interface.h"

namespace kagami {
  namespace management {
    ContainerPool &GetContainerPool();
    ObjectContainer &GetCurrentContainer();
    ObjectContainer &CreateContainer();
    Object *FindObject(string id);
    Object *CreateObject(string id, Object &object);
    Object *CreateObject(string id, Object &&object);
    void CreateInterface(Interface temp);
    void CreateGenericInterface(Interface temp);
    bool DisposeManager();
    bool HasTailTokenRequest(GenericToken token);
    Interface Order(string id, string type = kTypeIdNull, int size = -1);
    Interface GetGenericInterface(GenericToken token);
    Object *CreateConstantObject(string id, Object &object);
    Object *CreateConstantObject(string id, Object &&object);

    namespace type {
      extern string GetMethods(string name);
      extern void AddTemplate(string name, ObjectCopyingPolicy temp);
      extern shared_ptr<void> GetObjectCopy(Object &object);
    }
  }
}
