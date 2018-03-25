#pragma once
#ifndef _SE_INCLUDES_
#define _SE_INCLUDES_

#include <algorithm>
#include <string>
#include <vector>
#include <stack>
#include <fstream>
#include <deque>
#include <regex>
#include <utility>
#include <array>
#include "SDL.h"
#include "rebornkit.h"
#pragma comment(lib,"SDL2.lib")
#pragma comment(lib,"SDL2main.lib")

namespace Suzu {
  using std::ifstream;
  using std::ofstream;
  using std::string;
  using std::vector;
  using std::stack;
  using std::array;
  using std::deque;
  using std::regex;
  using std::pair;
  using std::to_string;
  using std::stoi;
  using std::stof;
  using std::stod;
  using std::regex_match;
}

#define _CANARY_FLAG_

#endif // !_SE_INCLUDES_

