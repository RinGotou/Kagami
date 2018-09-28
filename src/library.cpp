#include "library.h"

namespace kagami {
#if defined(_WIN32)
  Message LibHandlerConstructor(ObjectMap &p) {
    Object &objPath = p["path"];
    ObjTypeId typeId = objPath.GetTypeId();
    wstring path;
    if (typeId == kTypeIdRawString || typeId == kTypeIdString) {
      path = s2ws(Kit().GetRawString(GetObjectStuff<string>(objPath)));
    }
    else if (typeId == kTypeIdWideString) {
      path = GetObjectStuff<wstring>(objPath);
    }
    
    Message msg;
    HINSTANCE hIns = LoadLibrary(path.c_str());
    if (hIns != nullptr) {
      LibraryHandler handler(hIns);
      Object obj;
      obj.Set(make_shared<LibraryHandler>(handler), kTypeIdLib)
        .SetMethods(kLibraryMethods)
        .SetRo(false);
      msg.SetObject(obj, "__result");
    }
    else {
      msg.combo(kStrFatalError, kCodeIllegalCall, "Can't load library.");
    }
    return msg;
  }

  Message LibCall(ObjectMap &p) {
    Object &obj = p[kStrObject], 
      objId = p["id"], 
      objSize = p["__size"];

    HINSTANCE hIns = GetObjectStuff<LibraryHandler>(obj).Get();
    string id = Kit().GetRawString(GetObjectStuff<string>(objId));
    int size = stoi(GetObjectStuff<string>(objSize));

    char **argArray = 
      static_cast<char **>(malloc(sizeof(char *)*size));

    Message msg;
    string temp;

    bool state = true;
    for (int idx = 0; idx < size; ++idx) {
      Object &objArg = p["arg" + to_string(idx)];
      ObjTypeId typeId = objArg.GetTypeId();
      if (typeId != kTypeIdRawString && typeId != kTypeIdString) {
        state = false;
        msg.combo(kStrFatalError, kCodeIllegalCall, "Can't deliver object to library function except plain-type object.");
        break;
      }
      temp = GetObjectStuff<string>(objArg);
      argArray[idx] = static_cast<char *>(malloc(sizeof(char)*(temp.size() + 1)));
      strcpy(argArray[idx], temp.c_str());
      temp.clear();
    }

    if (state) {
      LibraryCallingFunc calling = LibraryCallingFunc(GetProcAddress(hIns, "Call"));
      LibraryGabageProc gabageProc = LibraryGabageProc(GetProcAddress(hIns, "GabageProc"));
      if (calling != nullptr && gabageProc != nullptr) {
        MessageBridge bridge = calling(id.c_str(), argArray, size);
        msg = Message(string(bridge.value), bridge.code, string(bridge.detail));
        gabageProc(bridge);
      }
    }

    for (size_t idx = 0; idx < size; ++idx) {
      free(*(argArray+(idx)));
    }
    free(argArray);

    return msg;
  }
#else

#endif
  void InitLibraryHandler() {
    using type::AddTemplate;
    using entry::AddEntry;
#if defined(_WIN32)
    AddTemplate(kTypeIdLib, ObjectPlanner(SimpleSharedPtrCopy<LibraryHandler>, kLibraryMethods));
    AddEntry(Entry(LibHandlerConstructor, kCodeNormalParm, "path", "library"));
    AddEntry(Entry(LibCall, kCodeAutoSize, "id|arg", "call", kTypeIdLib, kFlagMethod));
#else

#endif
  }
}