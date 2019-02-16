#pragma once
#include "module.h"

namespace kagami {
  /* Runtime strategy identifier for binary operator */
  enum PairTypePolicy { G_INT, G_FLOAT, G_STR, G_OTHER_OBJ };

  /* Identifier for binary operator */
  enum OperatorCode {
    PLUS, MINUS, TIMES, DIV, EQUAL, EQUALS,
    GREATER, LESS, NOT_EQUAL, GREATER_OR_EQUAL, LESS_OR_EQUAL,
    AND, OR, BIT_AND, BIT_OR,
    NUL
  };

  enum MonoOperatorCode {
    BIT_NOT, NOT
  };

  PairTypePolicy GetTypePolicy(Object &A, Object &B);

  /* Unified String Convertor Generator */
  template <class DestType>
  class StringConvertor {
  public:
    DestType Do(const string &str) {}
  };

  template <>
  class StringConvertor<long> {
  public:
    long Do(const string &str) { return stol(str); }
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

  ///////////////////////////////////////////////////////////////////
  /* 
    Binary Operator Function Generator for plain data(rawstring type)
  */
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
  class Operation<long, long, OperatorCode::TIMES> {
  public:
    long Do(long A, long B) { return A * B; }
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
  class Operation<long, long, OperatorCode::DIV> {
  public:
    long Do(long A, long B) { return A / B; }
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

  /* Result processing policy */
  template <class ResultType>
  class ResultPolicy {
  public:
    string Convert(ResultType src) {
      return to_string(src);
    }
  };

  template<>
  class ResultPolicy<string> {
  public:
    string Convert(string src) {
      return src;
    }
  };

  template<>
  class ResultPolicy<bool> {
  public:
    string Convert(bool src) {
      return src ? kStrTrue : kStrFalse;
    }
  };

  /* Binary operator function interface */
  class Action {
  public:
    virtual ~Action() {}
    virtual Message Do(Object &, Object &) = 0;
  };

  /* Unified operator function generator for plain string type(rawstring) */
  template<class ConvertorTargetType, OperatorCode op_code, class ResultType>
  class PlainStringAction : public Action {
  public:
    Message Do(Object &A, Object &B) override {
      StringConvertor<ConvertorTargetType> cvt;
      Operation<ConvertorTargetType, ResultType, op_code> op;
      ResultPolicy<ResultType> result_action;

      string data_A = A.Cast<string>();
      string data_B = B.Cast<string>();
      ResultType result = op.Do(cvt.Do(data_A), cvt.Do(data_B));
      string result_content = result_action.Convert(result);

      Message msg(result_content);

      return msg;
    }
  };

  /*
    Operator function generator for non-plain type.
    Base class returns false value for every operator that is not supported.
  */
  template<OperatorCode op_code>
  class ObjectAction : public Action {
  public:
    Message Do(Object &A, Object &B) override {
      return Message(kStrFalse);
    }
  };

  template<>
  class ObjectAction<OperatorCode::EQUALS> : public Action {
    Message Do(Object &A, Object &B) override {
      /*
        Find "__compare" method in object methods.
        If it's founded, Code will send objects to destination function.
      */
      bool result = find_in_vector<string>(kStrCompare,
        management::type::GetMethods(A.GetTypeId()));

      if (result) {
        ObjectMap obj_map = {
          NamedObject(kStrObject, A),
          NamedObject(kStrRightHandSide, B)
        };

        auto interface = management::FindInterface(kStrCompare, A.GetTypeId());
        return interface.Start(obj_map);
      }

      return Message(kStrFalse);
    }
  };

  template<>
  class ObjectAction<OperatorCode::NOT_EQUAL> : public Action {
    /*
      Same as above.
    */
    Message Do(Object &A, Object &B) override {
      bool result = find_in_vector<string>(kStrCompare,
        management::type::GetMethods(A.GetTypeId()));

      if (result) {
        ObjectMap obj_map = {
          NamedObject(kStrObject, A),
          NamedObject(kStrRightHandSide, B)
        };

        auto interface = management::FindInterface(kStrCompare, A.GetTypeId());
        /* 
          Reverse result.
        */
        string str = interface.Start(obj_map).GetObj().Cast<string>();
        if (str == kStrTrue) str = kStrFalse;
        else if (str == kStrFalse) str = kStrTrue;
        
        return Message(str);
      }

      return Message(kStrFalse);
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

  /* Top generator for binary operator */
  template<OperatorCode op_code, bool boolean_result>
  Message OperatorFunction(ObjectMap &p) {
    Message result;
    Object &A = p["first"];
    Object &B = p["second"];
    PairTypePolicy policy = GetTypePolicy(A, B);
    std::unique_ptr<Action> action;

    switch (policy) {
    case G_INT: action = std::make_unique<PlainStringAction<
        long, op_code, typename ResultTypeTraits<long, boolean_result>::ResultType>>();
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

  template <class Type, MonoOperatorCode code>
  class MonoAction {
  public:
    string Do(Type A) { return string(); }
  };

  template <>
  class MonoAction<string, NOT> {
  public:
    string Do(string A) {
      bool result;

      if (util::IsBoolean(A)) {
        if (A == kStrTrue) result = false;
        else if (A == kStrFalse) result = true;
      }
      else {
        result = false;
      }

      return util::MakeBoolean(result);
    }
  };

  template <>
  class MonoAction<long, NOT> {
  public:
    string Do(int A) {
      return util::MakeBoolean(A > 0);
    }
  };

  template <>
  class MonoAction<double, NOT> {
  public:
    string Do(double A) {
      return util::MakeBoolean(A > 0.0);
    }
  };

  template <>
  class MonoAction<long, BIT_NOT> {
  public:
    string Do(long A) {
      return to_string(~A);
    }
  };

  template <>
  class MonoAction<string, BIT_NOT> {
  public:
    string Do(string A) {
      string target = A;
      for (auto &unit : target) {
        unit = ~unit;
      }

      return target;
    }
  };

  template <MonoOperatorCode code>
  Message MonoOperatorFunction(ObjectMap &p) {
    Message result;
    Object &obj = p["first"];

    if (obj.GetTypeId() == kTypeIdRawString) {
      string raw = obj.Cast<string>();
      string result_str;
      TokenType type = util::GetTokenType(raw, true);

      switch (type) {
      case kTokenTypeInt:
        result_str = MonoAction<long, code>().Do(StringConvertor<long>().Do(raw));
        break;
      case kTokenTypeFloat:
        result_str = MonoAction<double, code>().Do(StringConvertor<double>().Do(raw));
        break;
      case kTokenTypeBool:
      case kTokenTypeString:
        result_str = MonoAction<string, code>().Do(raw);
        break;
      default:
        break;
      }

      result = Message(result_str);
    }
    else {
      result = Message(kCodeIllegalParam, "Invaild operation.", kStateError);
    }

    return result;
  }

  template<MonoOperatorCode op_code, GenericToken token>
  void MonoOperatorGenerator() {
    management::CreateGenericInterface(
      Interface(MonoOperatorFunction<op_code>, "first", token));
  }
}
