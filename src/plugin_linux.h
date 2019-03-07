#include "machine_kisaragi.h"

namespace kagami {
  struct PlainObject {
    void *data;
    int type;
  };

  class Plugin {
  private:
    void *handle;
  
  public:
    Plugin() : handle(nullptr) {}

    Plugin(string path) :
      handle(dlopen(path.c_str(),RTLD_LAZY)) {}

    bool Good() const { return handle == nullptr; }


  };
}
