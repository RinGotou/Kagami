#pragma once
#include "entry.h"

namespace kagami {
  namespace management {
    enum OperatorCode {
      ADD, SUB, MUL, DIV, EQUAL, IS,
      MORE, LESS, NOT_EQUAL, MORE_OR_EQUAL, LESS_OR_EQUAL,
      SELFINC, SELFDEC, AND, OR, NOT, BIT_AND, BIT_OR,
      NUL
    };

    OperatorCode GetOperatorCode(string src);

    using EntryMapUnit = map<string, Entry>::value_type;

    ContainerPool &GetContainerPool();
    ObjectContainer &GetCurrentContainer();
    ObjectContainer &CreateContainer();
    Object *FindObjectInCurrentContainer(string id);
    Object *FindObject(string id);
    Object *CreateObject(string id, Object &object);
    string GetTypeId(string id);
    void AddEntry(Entry temp);
    void AddGenericEntry(Entry temp);
    bool DisposeManager();
    bool HasTailTokenRequest(GenericTokenEnum token);
    Entry Order(string id, string type = kTypeIdNull, int size = -1);
    OperatorCode GetOperatorCode(string src);
    Entry GetGenericProvider(GenericTokenEnum token);

    namespace type {
      extern string GetMethods(string name);
      extern void AddTemplate(string name, ObjectCopyingPolicy temp);
      extern shared_ptr<void> GetObjectCopy(Object &object);
    }
  }
}
