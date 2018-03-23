#include "parser.h"
#define SDL_MAIN_HANDLED

int main(int argc, char **argv) {

  Suzu::Util util;

  util.ScriptStart("C:\\Apps\\main.rs");
  util.PrintEvents();

  return 0;
}