#pragma once
#include "parser.h"
#include "windows.h"

namespace Entry {
  using std::string;
  typedef vector<string> *(*Attachment)(void);
  //from MSDN
  std::wstring s2ws(const std::string& s);
  void ResetPlugin();

  class Instance : public pair<string, HINSTANCE> {
  private:
    bool health;
    vector<string> entrylist;
  public:
    Instance(string name, string path) {
      this->first = name;
      this->second = LoadLibrary(s2ws(path).c_str());
      if (second != nullptr) {
        health = true;

      }
      else {
        health = false;
      }
    }

    bool GetHealth() const { return health; }
  };
}

