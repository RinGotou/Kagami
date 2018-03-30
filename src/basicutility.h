#pragma once
#include "parser.h"


namespace Entry {
  typedef vector<string>* StrListPtr;
  typedef StrListPtr(*Attachment)(void);
  //from MSDN
  std::wstring s2ws(const std::string& s);
  void ResetPlugin();

  class Instance : public pair<string, HINSTANCE> {
  private:
    bool health;
    vector<string> entrylist;
  public:
    Instance(string name, string path);
    bool GetHealth() const { return health; }
    vector<string> &GetList() { return entrylist; }
  };
}

namespace Suzu {
  class MemoryProvider {
  private:
    deque<StrPair> dict;
    MemoryProvider *parent;
    typedef deque<StrPair>::iterator MemPtr;
  public:
    void create(StrPair unit) {
      if (unit.IsReadOnly()) {
        dict.push_front(unit);
      }
      else {
        dict.push_back(unit);
      }
    }

    StrPair *find(string name) {
      StrPair *result = nullptr;
      for (auto &unit : dict) {
        if (unit.first == name) {
          result = &unit;
          break;
        }
      }
      return result;
    }

    MemoryProvider &SetParent(MemoryProvider *ptr) {
      this->parent = ptr;
      return *this;
    }

    MemoryProvider *GetParent() const {
      return parent;
    }

    MemoryProvider() { parent = nullptr; }
    bool empty() const { return dict.empty(); }
    size_t size() const { return dict.size(); }
    void cleanup() { Util().CleanUpDeque(dict); }
    bool dispose(string name);
    string query(string name);
    string set(string name, string value);
  };

  void TotalInjection();
}

