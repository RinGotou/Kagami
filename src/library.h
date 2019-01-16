#pragma once
#include "module.h"

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
    HINSTANCE h_ins_;
    ref_blk *blk_;
  public:
    LibraryHandler(HINSTANCE h_ins_) { 
      this->h_ins_ = h_ins_; 
      blk_ = new ref_blk();
    }

    LibraryHandler(LibraryHandler &handler) {
      this->h_ins_ = handler.h_ins_;
      this->blk_ = handler.blk_;
      blk_->count += 1;
    }

    ~LibraryHandler() { 
      blk_->count -= 1;
      if (blk_->count == 0) {
        delete blk_;
        FreeLibrary(h_ins_);
      }
    }

    HINSTANCE Get() { return h_ins_; }
  };
#else

#endif
}