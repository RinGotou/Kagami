//BSD 2 - Clause License
//
//Copyright(c) 2017 - 2018, Suzu Nakamura
//All rights reserved.
//
//Redistribution and use in source and binary forms, with or without
//modification, are permitted provided that the following conditions are met :
//
//*Redistributions of source code must retain the above copyright notice, this
//list of conditions and the following disclaimer.
//
//* Redistributions in binary form must reproduce the above copyright notice,
//this list of conditions and the following disclaimer in the documentation
//and/or other materials provided with the distribution.
//
//THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
//AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
//IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
//DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
//FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
//DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
//  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
//  OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
//  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#include "kagamicommon.h"

namespace kagami {
  void Message::SetObject(Object &object, string id) {
    Attribute attribute;
    this->object = make_shared<Object>(object);
    this->code = kCodeObject;
    this->detail = id;
  }

  Object Message::GetObj() const {
    return *static_pointer_cast<Object>(this->object);
  }

  size_t Kit::GetDataType(string target) {
    using std::regex_match;
    size_t result;
    const auto match = [&](const regex &pat) -> bool {
      return regex_match(target, pat);
    };

    if (target == kStrNull || target == kStrEmpty) result = kTypeNull;
    else if (match(kPatternBoolean)) result = kTypeBoolean;
    else if (match(kPatternGenericToken)) result = kGenericToken;
    else if (match(kPatternInteger)) result = kTypeInteger;
    else if (match(kPatternDouble)) result = kTypeDouble;
    else if (match(kPatternSymbol)) result = kTypeSymbol;
    else if (match(kPatternBlank)) result = kTypeBlank;
    else if (IsString(target)) result = kTypeString;
    else result = kTypeNull;

    return result;
  }

  bool Kit::FindInStringVector(string target, string source) {
    bool result = false;
    auto methods = this->BuildStringVector(source);
    for (auto &unit : methods) {
      if (unit == target) {
        result = true;
        break;
      }
    }
    return result;
  }

  vector<string> Kit::BuildStringVector(string source) {
    vector<string> result;
    string temp;
    for (auto unit : source) {
      if (unit == '|') {
        result.push_back(temp);
        temp.clear();
        continue;
      }
      temp.append(1, unit);
    }
    if (temp != kStrEmpty) result.push_back(temp);
    return result;
  }

  Attribute Kit::GetAttrTag(string target) {
    Attribute result;
    vector<string> base;
    string temp;

    for (auto &unit : target) {
      if (unit == '+' || unit == '%') {
        if (temp == kStrEmpty) {
          temp.append(1, unit);
        }
        else {
          base.push_back(temp);
          temp = kStrEmpty;
          temp.append(1, unit);
        }
      }
      else {
        temp.append(1, unit);
      }
    }
    if (temp != kStrEmpty) base.push_back(temp);
    temp = kStrEmpty;

    for (auto &unit : base) {
      if (unit.front() == '%') {
        temp = unit.substr(1, unit.size() - 1);
        if (temp == kStrTrue) {
          result.ro = true;
        }
        else if (temp == kStrFalse) {
          result.ro = false;
        }
      }
      else if (unit.front() == '+') {
        temp = unit.substr(1, unit.size() - 1) + "|";
        result.methods.append(temp);
      }

      temp = kStrEmpty;
    }

    if (!result.methods.empty()) {
      if (result.methods.back() == '|') result.methods.pop_back();
    }

    return result;
  }

  string Kit::BuildAttrStr(Attribute target) {
    string result;
    auto methods = this->BuildStringVector(target.methods);
    if (target.ro)result.append("%true");
    else result.append("%false");
    if (!methods.empty()) {
      for (auto &unit : methods) {
        result.append("+" + unit);
      }
    }
    return result;
  }

  char Kit::ConvertChar(char target) {
    char result;
    switch (target) {
    case 't':result = '\t'; break;
    case 'n':result = '\n'; break;
    case 'r':result = '\r'; break;
    default:result = target; break;
    }
    return result;
  }

  wchar_t Kit::ConvertWideChar(wchar_t target) {
    wchar_t result;
    switch (target) {
    case L't':result = L'\t'; break;
    case L'n':result = L'\n'; break;
    case L'r':result = L'\r'; break;
    default:result = target; break;
    }
    return result;
  }

  bool Kit::IsWideString(string target) {
    auto result = false;
    for (auto &unit : target) {
      if (unit < 0 || unit>127) {
        result = true;
        break;
      }
    }
    return result;
  }
}