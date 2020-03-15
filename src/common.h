#pragma once
#if defined(_MSC_VER)
//Disable STUPID visual studio intellisense warning
#pragma warning(disable:4996)
#pragma warning(disable:6031)
#pragma warning(disable:6387)
#pragma warning(disable:26812)
#pragma warning(disable:26439)
#pragma warning(disable:26444)
#pragma warning(disable:26451)
#pragma warning(disable:26495)
#endif
#include <ctime>
#include <cstdio>
#include <clocale>
#include <cstdlib>

#include <string>
#include <utility>
#include <vector>
#include <memory>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <deque>
#include <stack>
#include <type_traits>
#include <functional>
#include <list>
#include <charconv>
#include <variant>
#include <filesystem>
#include <tuple>
#include <unordered_set>
#include <set>
#include <optional>

#include "toml11/toml.hpp"

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <direct.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

#include "dawn/src/dawn.ui.h"
#include "dawn/src/dawn.sound.h"
#include "minatsuki.log/src/minatsuki.log.h"

#define PRODUCT     "Kagami Project Core(KPC)"
#define PRODUCT_VER "2.5"
#define CODENAME    "Oratorio"
#define AUTHOR      "Rin Gotou"
#define COPYRIGHT   "Copyright(c) 2020"

namespace kagami {
  using std::string;
  using std::pair;
  using std::vector;
  using std::map;
  using std::set;
  using std::unordered_map;
  using std::unordered_set;
  using std::deque;
  using std::shared_ptr;
  using std::unique_ptr;
  using std::static_pointer_cast;
  using std::dynamic_pointer_cast;
  using std::make_shared;
  using std::make_unique;
  using std::make_pair;
  using std::size_t;
  using std::stack;
  using std::to_string;
  using std::stod;
  using std::stol;
  using std::wstring;
  using std::list;
  using std::initializer_list;
  using std::is_same;
  using std::is_base_of;
  using std::from_chars;
  using std::to_chars;
  using std::variant;
  using std::tuple;
  using std::optional;
  
  using minatsuki::StandardLogger;
  using minatsuki::StandardCachedLogger;
  using minatsuki::StandardRTLogger;
  namespace fs = std::filesystem;

#ifdef _WIN32
  const string kPlatformType = "Windows";
#else
#ifdef linux
  const string kPlatformType = "Linux";
#else
  const string kPlatformType = "POSIX Platform";
#endif
#endif

  /* Plain Type Code */
  enum PlainType {
    kPlainInt     = 1, 
    kPlainFloat   = 2, 
    kPlainString  = 3, 
    kPlainBool    = 4, 
    kNotPlainType = -1
  };

  //TODO:Port some object type to extension
  //int, float, bool, string, wstring, instream, outstream

  /* Embedded type identifier strings */
  const string kTypeIdNull            = "null";
  const string kTypeIdInt             = "int";
  const string kTypeIdFloat           = "float";
  const string kTypeIdBool            = "bool";
  const string kTypeIdString          = "string";
  const string kTypeIdWideString      = "wstring";
  const string kTypeIdArray           = "array";
  const string kTypeIdInStream        = "instream";
  const string kTypeIdOutStream       = "outstream";
  const string kTypeIdFunction        = "function";
  const string kTypeIdFunctionPointer = "function_pointer";
  const string kTypeIdObjectPointer   = "object_pointer";
  const string kTypeIdIterator        = "iterator";
  const string kTypeIdPair            = "pair";
  const string kTypeIdTable           = "table";
  const string kTypeIdStruct          = "struct";
  const string kTypeIdWindowEvent     = "window_event";
  const string kTypeIdWindow          = "window";
  const string kTypeIdElement         = "element";
  const string kTypeIdFont            = "font";
  const string kTypeIdTexture         = "texture";
  const string kTypeIdColorValue      = "color";
  const string kTypeIdRectangle       = "rectangle";
  const string kTypeIdPoint           = "point";
  const string kTypeIdModule          = "module";  
  const string kTypeIdExtension       = "extension";
  const string kTypeIdTOML            = "TOML";

  template <typename _Lhs, typename... _Rhs>
  inline bool compare(_Lhs lhs, _Rhs... rhs) {
    return ((lhs == rhs) || ...);
  }

  template <typename T>
  inline bool find_in_vector(T t, const vector<T> &&vec) {
    for (auto &unit : vec) {
      if (t == unit) {
        return true;
      }
    }

    return false;
  }

  template <typename T>
  inline bool find_in_vector(T t, const vector<T> &vec) {
    for (auto &unit : vec) {
      if (t == unit) {
        return true;
      }
    }

    return false;
  }

  template <typename _UnitType>
  inline bool find_in_list(_UnitType target, initializer_list<_UnitType> lst) {
    for (auto &unit : lst) {
      if (unit == target) return true;
    }
    return false;
  }
}

