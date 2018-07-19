/*
 * Akane - simple template kit 
 */
#pragma once
#include "akane_util.h"
#include "akane_list.h"
#include "akane_string.h"
#include "akane_stack.h"
#include "akane_stringbuilder.h"
#include "akane_vector.h"
#include <iostream>

extern std::ostream &operator<<(std::ostream &os, akane::string &str);

extern std::ostream &operator<<(std::ostream &os, akane::wstring &str);