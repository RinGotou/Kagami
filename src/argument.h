#pragma once
#include <string>
#include <utility>
#include <map>
#include <functional>
#include <initializer_list>
#include <vector>
#include <iostream>

namespace suzu {
  struct Option {
    bool has_value;
    bool optional;
    int group;

    Option(bool has_value, bool optional, int group = 0) :
      has_value(has_value),
      optional(optional),
      group(group) {}
  };

  using Parameters = std::map<std::string, Option>;
  using Pattern = Parameters::value_type;
  using Argument = std::pair<std::string, std::string>;
  using AnalyzedArgument = std::map<std::string, std::string>;

  enum HeadFormEnum {
    kHeadHorizon,
    kHeadDoubleHorizon, 
    kHeadSlash, 
    kHeadAll,
    kHeadNone
  };

  enum JoinerFormEnum {
    kJoinerColon,
    kJoinerEquals
  };

  enum ArgumentProcessorErrorEnum {
    kErrorBadHead,
    kErrorMisssingRequired,
    kErrorMissingValue,
    kErrorRedundantValue,
    kErrorRedundantArgument,
    kErrorUnknownArgument,
    kErrorEmptyPattern,
    kErrorNone
  };

  class ArgumentProcessorError {
  protected:
    std::string msg;

  public:
    ArgumentProcessorError(ArgumentProcessorErrorEnum e) {
      switch (e) {
      case kErrorBadHead:msg = "Argument head symbol is missing"; break;
      case kErrorMisssingRequired:msg = "Required argument is missing"; break;
      case kErrorMissingValue:msg = "Value of argument is missing"; break;
      case kErrorRedundantValue:msg = "Value of argument is redundant"; break;
      case kErrorRedundantArgument:msg = "Argument is redundant"; break;
      case kErrorUnknownArgument:msg = "Unknown argument"; break;
      case kErrorEmptyPattern:msg = "You must provide patterns"; break;
      default:break;
      }
    }

    std::string Report(std::string arg) {
      if (arg.empty()) {
        return msg;
      }

      return msg + " - " + arg;
    }
  };

  class StringGenerator {
  public:
    std::vector<std::string> output;

  public:
    StringGenerator() = delete;

    StringGenerator(int count, char **arr) {
      for (int idx = 1; idx < count; idx += 1) {
        output.emplace_back(std::string(arr[idx]));
      }
    }
  };

  template <HeadFormEnum head>
  class HeadChecker {};

  template <>
  class HeadChecker<kHeadAll> {
  public:
    std::string output;

    bool Do(std::string str) {
      bool result = false;

      if (str.substr(0, 2) == "--") {
        output = str.substr(2, str.size() - 2);
        result = true;
      }
      else if (str.front() == '/' || str.front() == '-') {
        output = str.substr(1, str.size() - 1);
        result = true;
      }

      return result;
    }
  };

  template <>
  class HeadChecker<kHeadNone> {
    bool Do(std::string str) { 
      return true; 
    }
  };

  template <>
  class HeadChecker<kHeadHorizon> {
  public:
    std::string output;

    bool Do(std::string str) {
      bool result = false;

      if (str.front() == '-') {
        output = str.substr(1, str.size() - 1);
        result = true;
      }

      return result;
    }
  };

  template <>
  class HeadChecker<kHeadDoubleHorizon> {
  public:
    std::string output;

    bool Do(std::string str) {
      bool result = false;

      if (str.substr(0, 2) == "--") {
        output = str.substr(2, str.size() - 2);
        result = true;
      }
      return result;
    }
  };

  template <>
  class HeadChecker<kHeadSlash> {
  public:
    std::string output;

    bool Do(std::string str) {
      bool result = false;

      if (str.front() == '/') {
        output = str.substr(1, str.size() - 1);
        result = true;
      }

      return result;
    }
  };

  template <JoinerFormEnum joiner>
  class JoinerPolicy {};

  template <>
  class JoinerPolicy<kJoinerColon> {
  public:
    char Joiner() {
      return ':';
    }
  };

  template <>
  class JoinerPolicy<kJoinerEquals> {
  public:
    char Joiner() {
      return '=';
    }
  };

  template <JoinerFormEnum joiner>
  class JoinerChecker {
  protected:
    JoinerPolicy<joiner> policy_;
   
  public:
    Argument value;

    bool Do(std::string str) {
      value = Argument();

      if (str.front() == policy_.Joiner()) {
        return false;
      }

      bool found = false;
      for (size_t idx = 0; idx < str.size(); idx += 1) {
        if (str[idx] == policy_.Joiner()) {
          if (idx < str.size() - 1) {
            value = Argument(
              str.substr(0, idx), 
              str.substr(idx + 1, str.size() - idx + 1)
            );
          }
          else {
            value = Argument(
              str.substr(0, str.size() - 1),
              std::string()
            );
          }
          found = true;
          break;
        }
      }

      if (!found) {
        value.first = str;
      }

      return found;
    }
  };

  template <HeadFormEnum head, JoinerFormEnum joiner>
  class ArgumentProcessor {
  protected:
    ArgumentProcessorErrorEnum error_;
    std::string bad_arg_;
    Parameters params_;
    HeadChecker<head> head_checker_;
    JoinerChecker<joiner> joiner_checker_;
    AnalyzedArgument analyzed_;
    
    bool CheckingAfterGenerating() {
      bool result = true;
      std::map<int, bool> has_value_checker;
      std::map<int, bool> optional_tracker; 

#define ERROR_CHECKING(_Cond, _Code) \
      if (_Cond){                    \
        error_ = _Code;              \
        bad_arg_ = unit.first;       \
        result = false;              \
        break;                       \
      }

      for (const auto &unit : params_) {
        if (unit.second.group == 0) {
          ERROR_CHECKING(!unit.second.optional
            && analyzed_.find(unit.first) == analyzed_.end(),
            kErrorMisssingRequired);
          continue;
        }

        optional_tracker[unit.second.group] = unit.second.optional;
        bool found = (analyzed_.find(unit.first) != analyzed_.end());

        auto it = has_value_checker.find(unit.second.group);

        if (it != has_value_checker.end()) {
          ERROR_CHECKING(it->second && found, kErrorRedundantArgument);

          if (!it->second && found) {
            it->second = true;
          }
        }
        else {
          has_value_checker[unit.second.group] = found;
        }
      }

      for (const auto &unit : has_value_checker) {
        ERROR_CHECKING(!unit.second && !optional_tracker[unit.first],
          kErrorMisssingRequired);
      }
      return result;
#undef ERROR_CHECKING
    }

  public:
    virtual ~ArgumentProcessor() {}

    ArgumentProcessor() :
      error_(kErrorNone),
      bad_arg_()  {}

    ArgumentProcessor(const std::initializer_list<Pattern> &&rhs) :
      error_(kErrorNone),
      bad_arg_(),
      params_(rhs) {}

    ArgumentProcessor &operator=(const std::initializer_list<Pattern> &rhs) {
      params_ = rhs;
      return *this;
    }

    ArgumentProcessor &operator=(const std::initializer_list<Pattern> &&rhs) {
      return this->operator=(rhs);
    }

    bool Generate(int argc,char **argv) {
      if (params_.empty()) {
        error_ = kErrorEmptyPattern;
        return false;
      }

      StringGenerator generator(argc, argv);
      bool result = true;
      analyzed_.clear();

#define ERROR_CHECKING(_Cond, _Code)  \
      if (_Cond) {                    \
          error_ = _Code;             \
          bad_arg_ = unit;            \
          result = false;             \
          break;                      \
      }

      for (const auto &unit : generator.output) {
        ERROR_CHECKING(!head_checker_.Do(unit), kErrorUnknownArgument);

        bool has_joiner = joiner_checker_.Do(head_checker_.output);
        auto it = params_.find(joiner_checker_.value.first);

        ERROR_CHECKING(it == params_.end(), kErrorUnknownArgument);

        ERROR_CHECKING(it->second.has_value &&
          (!has_joiner || (has_joiner && joiner_checker_.value.second.empty())),
          kErrorMissingValue);

        ERROR_CHECKING(!it->second.has_value && has_joiner, kErrorRedundantValue);

        if (it->second.has_value && has_joiner) {
          analyzed_.insert(joiner_checker_.value);
        }
        else if (!it->second.has_value && !has_joiner) {
          analyzed_.insert(AnalyzedArgument::value_type(head_checker_.output, ""));
        }          
      }

      if (!result) {
        analyzed_.clear();
      }
      else {
        result = CheckingAfterGenerating();
      }

      return result;
#undef ERROR_CHECKING
    }

    bool Exist(std::string str) const {
      return analyzed_.find(str) != analyzed_.end();
    }

    std::string ValueOf(std::string key) const {
      return analyzed_.find(key)->second;
    }

    std::string BadArg() const {
      return bad_arg_;
    }

    ArgumentProcessorErrorEnum Error() const {
      return error_;
    }

    bool Good() const {
      return error_ == kErrorNone;
    }
  };
}