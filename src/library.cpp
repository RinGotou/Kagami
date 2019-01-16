#include "library.h"

namespace kagami {
#if defined(_WIN32)
  Message LibHandlerConstructor(ObjectMap &p) {
    wstring path;
    Message msg;

    if (p.CheckTypeId("path", IsStringObject)) {
      path = s2ws(util::GetRawString(p.Get<string>("path")));
    }
    else if (p.CheckTypeId("path", kTypeIdWideString)) {
      path = p.Get<wstring>("path");
    }
    
    
    HINSTANCE h_ins = LoadLibrary(path.c_str());
    if (h_ins != nullptr) {
      LibraryHandler handler(h_ins);
      Object obj;
      obj.Set(make_shared<LibraryHandler>(handler), kTypeIdLib)
        .SetMethods(kLibraryMethods);
      msg.SetObject(obj);
    }
    else {
      msg = Message(kStrFatalError, kCodeIllegalCall, "Can't load library.");
    }
    return msg;
  }

  /*so dirty...*/
  Message LibCall(ObjectMap &p) {
    LibraryHandler &handler = p.Get<LibraryHandler>(kStrObject);
    string id = RealString(p.Get<string>("id"));
    int size = p.GetVaSize();
    bool state = true;
    HINSTANCE h_ins = handler.Get();

    char **argArray = 
      static_cast<char **>(malloc(sizeof(char *)*size));

    Message msg;
    string temp;

    for (int idx = 0; idx < size; ++idx) {
      string sub = "arg" + to_string(idx);

      if (!p.CheckTypeId(sub, IsStringObject)) {
        state = false;
        msg = Message(kStrFatalError, kCodeIllegalCall, 
          "Can't deliver object to library function except plain-type object.");
        break;
      }

      temp = p.Get<string>(sub);
      argArray[idx] = static_cast<char *>(malloc(sizeof(char)*(temp.size() + 1)));
      strcpy(argArray[idx], temp.c_str());
      temp.clear();
    }

    if (state) {
      LibraryCallingFunc calling = 
        LibraryCallingFunc(GetProcAddress(h_ins, "Call"));
      LibraryGabageProc gabageProc = 
        LibraryGabageProc(GetProcAddress(h_ins, "GabageProc"));

      if (calling != nullptr && gabageProc != nullptr) {
        MessageBridge bridge = calling(id.c_str(), argArray, size);
        if (bridge.value == kStrRedirect) {
          msg = Message(string(bridge.detail));
        }
        else {
          msg = Message(string(bridge.value), bridge.code, string(bridge.detail));
        }
        

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
    using management::type::AddTemplate;
    using management::AddEntry;
#if defined(_WIN32)
    AddTemplate(kTypeIdLib, ObjectCopyingPolicy(SimpleSharedPtrCopy<LibraryHandler>, kLibraryMethods));
    AddEntry(Entry(LibHandlerConstructor, "path", "library"));
    AddEntry(Entry(LibCall, "id|arg", "call", kTypeIdLib, kCodeAutoSize));
#else

#endif
  }
}