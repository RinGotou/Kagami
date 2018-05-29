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
#include <memory>
#include <map>
#include <deque>
#include <regex>

namespace kagami {
  using std::string;
  using std::pair;
  using std::vector;
  using std::map;
  using std::deque;
  using std::shared_ptr;
  using std::static_pointer_cast;
  using std::regex;

  struct ActivityTemplate;
  class Message;
  class ObjTemplate;
  class Object;

  using ObjectMap = map<string, Object>;
  using CopyCreator = shared_ptr<void>(*)(shared_ptr<void>);
  using CastFunc = pair<string, CopyCreator>;
  using Activity = Message(*)(ObjectMap &);
  using CastAttachment = map<string, ObjTemplate> *(*)();
  using MemoryDeleter = int(*)(void *);
  using Attachment = vector<ActivityTemplate> * (*)(void);


#if defined(_WIN32)
  const string kEngineVersion = "version 0.5 (Windows Platform)";
#else
  const string kEngineVersion = "version 0.5 (Linux Platform)";
#endif
  const string kEngineName = "Kagami";
  const string kEngineAuthor = "Suzu Nakamura";
  const string kCopyright = "Copyright(c) 2017-2018";
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

  const int kCodeAutoFill = 14;
  const int kCodeNormalArgs = 13;
  const int kCodeFillingSign = 12;
  const int kCodeReturn = 11;
  const int kCodeConditionLeaf = 10;
  const int kCodeConditionBranch = 9;
  const int kCodeConditionRoot = 8;
  const int kCodeObject = 7;
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
  const int kFlagMethod = 3;

  const size_t kTypeFunction = 0;
  const size_t kTypeString = 1;
  const size_t kTypeInteger = 2;
  const size_t kTypeDouble = 3;
  const size_t kTypeBoolean = 4;
  const size_t kTypeSymbol = 5;
  const size_t kTypeBlank = 6;
  const size_t kTypeChar = 7;
  const size_t kTypeNull = 100;

  const string kTypeIdNull = "null";
  const string kTypeIdInt = "int";
  const string kTypeIdRawString = "string";
  const string kTypeIdArrayBase = "deque";
  const string kTypeIdCubeBase = "cube";
  const string kTypeIdRef = "__ref";

  const size_t kModeNormal = 0;
  const size_t kModeNextCondition = 1;
  const size_t kModeCycle = 2;
  const size_t kModeCycleJump = 3;

  const regex kPatternFunction(R"([a-zA-Z_][a-zA-Z_0-9]*)");
  const regex kPatternNumber(R"(\d+\.?\d*)");
  const regex kPatternInteger(R"([-]?\d+)");
  const regex kPatternDouble(R"([-]?\d+\.\d+)");
  const regex kPatternBoolean(R"(\btrue\b|\bfalse\b)");
  const regex kPatternSymbol(R"(==|<=|>=|!=|&&|\|\||[[:Punct:]])");
  const regex kPatternBlank(R"([[:blank:]])");

  /*Object Tag Struct
    no description yet.
  */
  struct AttrTag {
    string methods;
    bool ro;
    AttrTag(string methods, bool ro) {
      this->methods = methods;
      this->ro = ro;
    }
    AttrTag(){}
  };

  /*Activity Template class
    no description yet.
  */
  struct ActivityTemplate {
    string id;
    Activity activity;
    int priority;
    int arg_mode;
    string args;
    string specifictype;

    ActivityTemplate &set(string id, Activity activity, int priority, int arg_mode, string args,string type = kTypeIdNull) {
      this->id = id;
      this->activity = activity;
      this->priority = priority;
      this->arg_mode = arg_mode;
      this->args = args;
      this->specifictype = type;
      return *this;
    }
  };

  /* Object Template Class
  this class contains custom class info for script language.
  */
  class ObjTemplate {
  private:
    CopyCreator copyCreator;
    string methods;
  public:
    ObjTemplate() : methods(kStrEmpty) {
      copyCreator = nullptr;
    }
    ObjTemplate(CopyCreator copyCreator, string methods) {
      this->copyCreator = copyCreator;
      this->methods = methods;
    }

    shared_ptr<void> CreateObjectCopy(shared_ptr<void> target) {
      shared_ptr<void> result = nullptr;
      if (target != nullptr) {
        result = copyCreator(target);
      }
      return result;
    }
    string GetMethods() const {
      return methods;
    }
  };

  /*Message Class
    It's the basic message tunnel of this script processor.
    According to my design,processor will check value or detail or
    both of them to find out warnings or errors.Some functions use 
    value,detail and castpath to deliver Object class.
  */
  class Message {
  private:
    string value;
    string detail;
    int code;
    shared_ptr<void> ptr;
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
    shared_ptr<void> &GetPtr() { return ptr; }
  };

  /*Kit Class
  this class contains many useful template or tiny function, and
  create script processing workspace.
  */
  class Kit {
  public:
    template <class Type>
    Kit CleanupVector(vector<Type> &target) {
      target.clear();
      vector<Type>(target).swap(target);
      return *this;
    }

    template <class Type>
    Kit CleanupDeque(deque<Type> &target) {
      target.clear();
      deque<Type>(target).swap(target);
      return *this;
    }

    template <class Type>
    bool Compare(Type source, vector<Type> list) {
      bool result = false;
      for (auto unit : list) {
        if (unit == source) {
          result = true;
          break;
        }
      }
      return result;
    }

    template <class Type>
    Type Calc(Type A, Type B, string opercode) {
      Type result = 0;
      if (opercode == "+") result = A + B;
      if (opercode == "-") result = A - B;
      if (opercode == "*") result = A * B;
      if (opercode == "/") result = A / B;
      return result;
    }

    template <class Type>
    bool Logic(Type A, Type B, string opercode) {
      bool result = false;
      if (opercode == "==") result = (A == B);
      if (opercode == "<=") result = (A <= B);
      if (opercode == ">=") result = (A >= B);
      if (opercode == "!=") result = (A != B);
      return result;
    }

    string GetRawString(string target) {
      return target.substr(1, target.size() - 2);
    }

    int GetDataType(string target);
    AttrTag GetAttrTag(string target);
    string MakeAttrTagStr(AttrTag target);
    bool FindInStringVector(string target, string source);
    vector<string> BuildStringVector(string source);
  };

}

