#pragma once
#include "parser.h"


namespace Entry {
  using std::shared_ptr;
  typedef vector<string>* StrListPtr;
  typedef StrListPtr(*Attachment)(void);
  //from MSDN
  std::wstring s2ws(const std::string& s);
  //void ResetPlugin();

  class Instance : public pair<string, HINSTANCE> {
  private:
    bool health;
    vector<string> entrylist;
  public:
    Instance() { health = false; }
    bool Load(string name, HINSTANCE h);
    bool GetHealth() const { return health; }
    vector<string> &GetList() { return entrylist; }
  };
}

