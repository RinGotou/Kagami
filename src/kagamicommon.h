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
#pragma once
#include <string>
#include <utility>
#include <vector>
#include <cstring>
#include <memory>
#include <map>
#include <deque>

namespace Kagami {
  using std::string;
  using std::pair;
  using std::vector;
  using std::shared_ptr;
  using std::static_pointer_cast;
  using std::map;

#if defined(_WIN32)
  const string kEngineVersion = "version 0.3 (Windows Platform)";
#else
  const string kEngineVersion = "version 0.3 (UNIX-like Platform)";
#endif
  const string kEngineName = "Kagami";
  const string kEngineAuthor = "Suzu Nakamura";
  const string kCopyright = "Conpyright(c) 2017-2018";
  const string kStrDefineCmd = "var";
  const string kStrSetCmd = "__set";
  const string kStrNull = "null";

  const string kStrEmpty = "";
  const string kStrFatalError = "__FATAL__";
  const string kStrWarning = "__WARNING__";
  const string kStrSuccess = "__SUCCESS__";
  const string kStrEOF = "__EOF__";
  const string kStrPass = "__PASS__";
  const string kStrRedirect = "__*__";
  const string kStrTrue = "true";
  const string kStrFalse = "false";

  const int kCodeFillingSign = 12;
  const int kCodeReturn = 11;
  const int kCodeConditionLeaf = 10;
  const int kCodeConditionBranch = 9;
  const int kCodeConditionRoot = 8;
  const int kCodePoint = 7;
  const int kCodeTailSign = 5;
  const int kCodeHeadSign = 4;
  const int kCodeQuit = 3;
  const int kCodeRedirect = 2;
  const int kCodeNothing = 1;
  const int kCodeSuccess = 0;
  const int kCodeBrokenEntry = -1;
  const int kCodeOverflow = -2;
  const int kCodeIllegalArgs = -3;
  const int kCodeIllegalCall = -4;
  const int kCodeIllegalSymbol = -5;
  const int kCodeBadStream = -6;
  const int kFlagCoreEntry = 0;
  const int kFlagNormalEntry = 1;
  const int kFlagBinEntry = 2;
  const int kFlagPluginEntry = 3;
  const int kFlagAutoSize = -1;
  const int kFlagAutoFill = -2;
  const int kFlagNotDefined = -3;

  const size_t kTypeFunction = 0;
  const size_t kTypeString = 1;
  const size_t kTypeInteger = 2;
  const size_t KTypeDouble = 3;
  const size_t kTypeBoolean = 4;
  const size_t kTypeSymbol = 5;
  const size_t kTypeBlank = 6;
  const size_t kTypeNull = 100;

  const size_t kModeNormal = 0;
  const size_t kModeNextCondition = 1;
  const size_t kModeCycle = 2;
  const size_t kModeCycleJump = 3;

  /*Message Class
    It's the basic message tunnel of this script processor.
    According to my design,processor will check value or detail or
    both of them to find out warnings or errors.Some function use 
    value,detail and castpath to deliver PointWrapper class.
  */
  class Message {
  private:
    string value;
    string detail;
    int code;
    shared_ptr<void> castpath;
  public:
    Message() {
      value = kStrEmpty;
      code = kCodeSuccess;
      detail = kStrEmpty;
    }

    Message(string value, int code, string detail) {
      this->value = value;
      this->code = code;
      this->detail = detail;
    }

    Message combo(string value, int code, string detail) {
      this->value = value;
      this->code = code;
      this->detail = detail;
      return *this;
    }

    Message SetValue(const string &value) {
      this->value = value;
      return *this;
    }

    Message SetCode(const int &code) {
      this->code = code;
      return *this;
    }

    Message SetDetail(const string &detail) {
      this->detail = detail;
      return *this;
    }

    string GetValue() const { return this->value; }
    int GetCode() const { return this->code; }
    string GetDetail() const { return this->detail; }
    shared_ptr<void> &GetCastPath() { return castpath; }
  };

  typedef map<string, shared_ptr<void>> PathMap;
  typedef map<string, string> StrMap;
  typedef shared_ptr<void>(*CastTo)(shared_ptr<void>);
  typedef pair<string, CastTo> CastFunc;
  typedef void *(*CastToEx)(shared_ptr<void> &);
  typedef Message(*Activity)(PathMap &);
  typedef Message *(*PluginActivity)(PathMap &);
  typedef map<string, CastToEx> *(*CastAttachment)();
  typedef void(*MemoryDeleter)(void *);
}
