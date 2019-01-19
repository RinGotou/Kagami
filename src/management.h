#pragma once
#include "entry.h"

namespace kagami {
  namespace management {
    ContainerPool &GetContainerPool();
    ObjectContainer &GetCurrentContainer();
    ObjectContainer &CreateContainer();
    Object *FindObject(string id);
    Object *CreateObject(string id, Object &object);
    Object *CreateObject(string id, Object &&object);
    void AddEntry(Entry temp);
    void AddGenericEntry(Entry temp);
    bool DisposeManager();
    bool HasTailTokenRequest(GenericTokenEnum token);
    Entry Order(string id, string type = kTypeIdNull, int size = -1);
    Entry GetGenericProvider(GenericTokenEnum token);
    Object *CreateConstantObject(string id, Object &object);
    Object *CreateConstantObject(string id, Object &&object);

    namespace type {
      extern string GetMethods(string name);
      extern void AddTemplate(string name, ObjectCopyingPolicy temp);
      extern shared_ptr<void> GetObjectCopy(Object &object);
    }
  }
}
