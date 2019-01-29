#pragma once
#include "module.h"

namespace kagami {
  enum PairTypePolicy { G_INT, G_FLOAT, G_STR, G_OTHER_OBJ };

  enum OperatorCode {
    PLUS, MINUS, TIMES, DIV, EQUAL, EQUALS,
    GREATER, LESS, NOT_EQUAL, GREATER_OR_EQUAL, LESS_OR_EQUAL,
    AND, OR, NOT, BIT_AND, BIT_OR,
    NUL
  };

  PairTypePolicy GetTypePolicy(Object &A, Object &B);

  /* Unified String Convertor Generator */
  template <class DestType>
  class StringConvertor {
  public:
    DestType Do(const string &str) {}
  };

  template <>
  class StringConvertor<int> {
  public:
    int Do(const string &str) { return stoi(str); }
  };

  template <>
  class StringConvertor<double> {
  public:
    double Do(const string &str) { return stod(str); }
  };

  template<>
  class StringConvertor<string> {
  public:
    string Do(const string &str) { return str; }
  };

  /* Binary Operator Generator */
  template <class Type, class DestType, OperatorCode op_code>
  class Operation {
  public:
    DestType Do(Type A, Type B) { return DestType(); }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::EQUALS> {
  public:
    bool Do(Type A, Type B) { return A == B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::LESS_OR_EQUAL> {
  public:
    bool Do(Type A, Type B) { return A <= B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::GREATER_OR_EQUAL> {
  public:
    bool Do(Type A, Type B) { return A >= B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::NOT_EQUAL> {
  public:
    bool Do(Type A, Type B) { return A != B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::GREATER> {
  public:
    bool Do(Type A, Type B) { return A > B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::LESS> {
  public:
    bool Do(Type A, Type B) { return A < B; }
  };

  template<class Type, class DestType>
  class Operation<Type, DestType, OperatorCode::PLUS> {
  public:
    DestType Do(Type A, Type B) { return A + B; }
  };

  template<class Type, class DestType>
  class Operation<Type, DestType, OperatorCode::MINUS> {
  public:
    DestType Do(Type A, Type B) { return A - B; }
  };

  template<>
  class Operation<string, string, OperatorCode::MINUS> {
  public:
    string Do(string A, string B) { return string(); }
  };

  template<class Type, class DestType>
  class Operation<Type, DestType, OperatorCode::TIMES> {
  public:
    DestType Do(Type A, Type B) { return A * B; }
  };

  template<>
  class Operation<int, int, OperatorCode::TIMES> {
  public:
    int Do(int A, int B) { return A * B; }
  };

  template<>
  class Operation<double, double, OperatorCode::TIMES> {
  public:
    double Do(double A, double B) { return A * B; }
  };

  template<>
  class Operation<string, string, OperatorCode::TIMES> {
  public:
    string Do(string A, string B) { return string(); }
  };

  template<class Type, class DestType>
  class Operation<Type, DestType, OperatorCode::DIV> {
  public:
    DestType Do(Type A, Type B) { return DestType(); }
  };

  template<>
  class Operation<int, int, OperatorCode::DIV> {
  public:
    int Do(int A, int B) { return A / B; }
  };

  template<>
  class Operation<double, double, OperatorCode::DIV> {
  public:
    double Do(double A, double B) { return A / B; }
  };

  template<>
  class Operation<string, string, OperatorCode::DIV> {
  public:
    string Do(string A, string B) { return string(); }
  };

  template<>
  class Operation<string, string, OperatorCode::PLUS> {
  public:
    string Do(string A, string B) {
      string temp;
      if (A.front() != '\'') A = "'" + A;
      if (A.back() == '\'')  A = A.substr(0, A.size() - 1);
      if (B.front() == '\'') B = B.substr(1, B.size() - 1);
      if (B.back() != '\'')  B = B + "'";
      temp = A + B;
      return temp;
    }
  };

  template<>
  class Operation<string, bool, OperatorCode::AND> {
  public:
    bool Do(string A, string B) {
      bool result;

      if (util::IsBoolean(A) && util::IsBoolean(B)) {
        result = A == kStrTrue && B == kStrTrue;
      }
      else {
        result = false;
      }

      return result;
    }
  };

  template<>
  class Operation<string, bool, OperatorCode::OR> {
  public:
    bool Do(string A, string B) {
      bool result;

      if (util::IsBoolean(A) && util::IsBoolean(B)) {
        result = A == kStrTrue || B == kStrTrue;
      }
      else {
        result = false;
      }

      return result;
    }
  };

  template <class ResultType>
  class ResultAction {
  public:
    string Convert(ResultType src) {
      return to_string(src);
    }
  };

  template<>
  class ResultAction<string> {
  public:
    string Convert(string src) {
      return src;
    }
  };

  template<>
  class ResultAction<bool> {
  public:
    string Convert(bool src) {
      return src ? kStrTrue : kStrFalse;
    }
  };

  class Action {
  public:
    virtual Message Do(Object &, Object &) = 0;
  };

  template<class ConvertorTargetType, OperatorCode op_code, class ResultType>
  class PlainStringAction : public Action {
  public:
    Message Do(Object &A, Object &B) override {
      StringConvertor<ConvertorTargetType> cvt;
      Operation<ConvertorTargetType, ResultType, op_code> op;
      ResultAction<ResultType> result_action;

      string data_A = A.Cast<string>();
      string data_B = B.Cast<string>();
      ResultType result = op.Do(cvt.Do(data_A), cvt.Do(data_B));
      string result_content = result_action.Convert(result);

      Message msg(result_content);

      return msg;
    }
  };

  template<OperatorCode op_code>
  class ObjectAction : public Action {
  public:
    Message Do(Object &A, Object &B) override {
      //TODO:
      return Message();
    }
  };

  template<class Type, bool boolean_result>
  class ResultTypeTraits {};

  template<class Type>
  class ResultTypeTraits<Type, true> {
  public:
    using ResultType = bool;
  };

  template<class Type>
  class ResultTypeTraits<Type, false> {
  public:
    using ResultType = Type;
  };

  template<OperatorCode op_code, bool boolean_result>
  Message OperatorFunction(ObjectMap &p) {
    Message result;
    Object &A = p["first"];
    Object &B = p["second"];
    PairTypePolicy policy = GetTypePolicy(A, B);
    std::unique_ptr<Action> action;

    switch (policy) {
    case G_INT: action = std::make_unique<PlainStringAction<
        int, op_code, typename ResultTypeTraits<int, boolean_result>::ResultType>>();
      break;
    case G_FLOAT: action = std::make_unique<PlainStringAction<
        double, op_code, typename ResultTypeTraits<double, boolean_result>::ResultType>>();
      break;
    case G_STR: action = std::make_unique<PlainStringAction<
      string, op_code, typename ResultTypeTraits<string, boolean_result>::ResultType>>();
      break;
    case G_OTHER_OBJ: action = std::make_unique<ObjectAction<op_code>>();
      break;
    default:
      break;
    }

    result = action->Do(A, B);

    return result;
  }

  template<OperatorCode op_code, GenericToken token, bool boolean_result = false>
  void OperatorGenerator() {
    management::CreateGenericInterface(
      Interface(OperatorFunction<op_code, boolean_result>, "first|second", token));
  }
}
