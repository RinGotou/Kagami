#include "parser.h"
//#define SDL_MAIN_HANDLED
//#define _DEBUG_STATE_

int main(int argc, char **argv) {

  Suzu::Util util;
#ifdef _DEBUG_STATE_
  util.ScriptStart("C:\\Apps\\main.rs");
#else
  if (argc > 1) {
    //load extern script
    util.ScriptStart(argv[1]);
  }
  else {
    //open terminal mode
    util.Terminal();
  }
#endif
  util.PrintEvents();

  return 0;
}