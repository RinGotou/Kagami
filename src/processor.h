#pragma once
#include "object.h"
#include "message.h"
#include "entry.h"

namespace kagami {
  using Inst = pair<Entry, deque<Object>>;

  class Processor {
    bool health;
    vector<Inst> instBase;
    size_t index;
    Message Run();
    Token mainToken;
  public:
    Processor() : health(false), index(0) {}
    Processor(vector<Inst> instBase, size_t index = 0, Token mainToken = Token()) : health(true),
      index(index) {
      this->instBase = instBase;
      this->mainToken = mainToken;
    }
    
    Message Activiate(size_t mode = kModeNormal);
    size_t GetIndex() const { return index; }
    bool IsHealth() const { return health; }
  };
}