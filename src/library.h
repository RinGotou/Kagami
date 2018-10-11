#pragma once
#include "machine.h"

namespace kagami {
  const string kLibraryMethods = "call";

  using MessageBridge = struct {
    char *value;
    int code;
    char *detail;
  };

  using LibraryCallingFunc = MessageBridge(*)(const char *, char **, int);
  using LibraryGabageProc = void(*)(MessageBridge);

#if defined(_WIN32)
  class LibraryHandler {
  private:
    HINSTANCE hIns;
    ref_blk *blk;
  public:
    LibraryHandler(HINSTANCE hIns) { 
      this->hIns = hIns; 
      blk = new ref_blk();
    }

    LibraryHandler(LibraryHandler &handler) {
      this->hIns = handler.hIns;
      this->blk = handler.blk;
      blk->count += 1;
    }

    ~LibraryHandler() { 
      blk->count -= 1;
      if (blk->count == 0) {
        delete blk;
        FreeLibrary(hIns);
      }
    }

    HINSTANCE Get() { return hIns; }
  };
#else

#endif
}