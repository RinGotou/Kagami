#pragma once
#include "parser.h"
#include "windows.h"

namespace Entry {
  using namespace Suzu;
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

