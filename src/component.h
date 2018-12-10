#pragma once
#include "machine.h"

namespace kagami {
  enum GroupTypeEnum { G_INT, G_FLOAT, G_STR, G_NUL };

  using OperatorCode = entry::OperatorCode;

  string IncAndDecOperation(Object &obj, bool negative, bool keep);
  GroupTypeEnum GetGroupType(Object &A, Object &B);

  template <bool negative, bool keep>
  Message SelfOperator(ObjectMap &p) {
    Object &obj = p["object"];
    //TODO:error
    string result = IncAndDecOperation(obj, negative, keep);
    return Message(result);
  }

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
  class Operation<Type, bool, OperatorCode::IS> {
  public:
    bool Do(Type A, Type B) { return A == B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::LESS_OR_EQUAL> {
  public:
    bool Do(Type A, Type B) { return A <= B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::MORE_OR_EQUAL> {
  public:
    bool Do(Type A, Type B) { return A >= B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::NOT_EQUAL> {
  public:
    bool Do(Type A, Type B) { return A != B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::MORE> {
  public:
    bool Do(Type A, Type B) { return A > B; }
  };

  template <class Type>
  class Operation<Type, bool, OperatorCode::LESS> {
  public:
    bool Do(Type A, Type B) { return A < B; }
  };

  template<class Type, class DestType>
  class Operation<Type, DestType, OperatorCode::ADD> {
  public:
    DestType Do(Type A, Type B) { return A + B; }
  };

  template<class Type, class DestType>
  class Operation<Type, DestType, OperatorCode::SUB> {
  public:
    DestType Do(Type A, Type B) { return A - B; }
  };

  template<>
  class Operation<string, string, OperatorCode::SUB> {
  public:
    string Do(string A, string B) { return string(); }
  };

  template<class Type, class DestType>
  class Operation<Type, DestType, OperatorCode::MUL> {
  public:
    DestType Do(Type A, Type B) { return A * B; }
  };

  template<>
  class Operation<int, int, OperatorCode::MUL> {
  public:
    int Do(int A, int B) { return A * B; }
  };

  template<>
  class Operation<double, double, OperatorCode::MUL> {
  public:
    double Do(double A, double B) { return A * B; }
  };

  template<>
  class Operation<string, string, OperatorCode::MUL> {
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
  class Operation<string, string, OperatorCode::ADD> {
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
  class Operation<string, string, OperatorCode::NOT_EQUAL> {
  public:
    string Do(string A, string B) {
      Operation<string, bool, OperatorCode::NOT_EQUAL> op;
      string result_str;
      util::MakeBoolean(op.Do(A, B), result_str);
      return result_str;
    }
  };

  template<>
  class Operation<string, string, OperatorCode::IS> {
  public:
    string Do(string A, string B) {
      Operation<string, bool, OperatorCode::IS> op;
      string result_str;
      util::MakeBoolean(op.Do(A, B), result_str);
      return result_str;
    }
  };

  template<>
  class Operation<string, string, OperatorCode::AND> {
  public:
    string Do(string A, string B) {
      string result_str;
      if (util::IsBoolean(A) && util::IsBoolean(B)) {
        util::MakeBoolean(A == kStrTrue && B == kStrTrue, result_str);
      }
      else {
        result_str = kStrFalse;
      }
      return result_str;
    }
  };

  template<>
  class Operation<string, string, OperatorCode::OR> {
  public:
    string Do(string A, string B) {
      string result_str;
      if (util::IsBoolean(A) && util::IsBoolean(B)) {
        util::MakeBoolean(A == kStrTrue || B == kStrTrue, result_str);
      }
      else {
        result_str = kStrFalse;
      }
      return result_str;
    }
  };

  template<class SrcType,OperatorCode op_code>
  class CalcBase {
  public:
    string Do(string data_A, string data_B) {
      StringConvertor<SrcType> cvt;
      Operation<SrcType, SrcType, op_code> op;
      return to_string(op.Do(cvt.Do(data_A), cvt.Do(data_B)));
    }
  };

  template <class SrcType, OperatorCode op_code>
  class LogicBase {
  public:
    string Do(string data_A, string data_B) {
      StringConvertor<SrcType> cvt;
      Operation<SrcType, bool, op_code> op;
      string result_str;
      bool result = op.Do(cvt.Do(data_A), cvt.Do(data_B));
      result ?
        result_str = kStrTrue :
        result_str = kStrFalse;
      return result_str;
    }
  };

  template <OperatorCode op_code>
  class StringBase {
  public:
    string Do(string data_A, string data_B) {
      Operation<string, string, op_code> op;
      string result_str = op.Do(data_A, data_B);
      return result_str;
    }
  };

  template <GroupTypeEnum group_type ,OperatorCode op_code>
  class GroupCalcBase {
  public:
    string Do(string A, string B) {}
  };

  template <GroupTypeEnum group_type, OperatorCode op_code>
  class GroupLogicBase {
  public:
    string Do(string A, string B) {}
  };

  template<OperatorCode op_code>
  class GroupCalcBase<G_INT, op_code> {
  public:
    string Do(string A, string B) {
      CalcBase<int, op_code> base;
      return base.Do(A, B);
    }
  };

  template<OperatorCode op_code>
  class GroupCalcBase<G_FLOAT, op_code> {
  public:
    string Do(string A, string B) {
      CalcBase<double, op_code> base;
      return base.Do(A, B);
    }
  };

  template<OperatorCode op_code> 
  class GroupCalcBase<G_STR, op_code> {
  public:
    string Do(string A, string B) {
      StringBase<op_code> base;
      return base.Do(A, B);
    }
  };

  template<OperatorCode op_code>
  class GroupLogicBase<G_INT, op_code> {
  public:
    string Do(string A, string B) {
      LogicBase<int, op_code> base;
      return base.Do(A, B);
    }
  };

  template<OperatorCode op_code>
  class GroupLogicBase<G_FLOAT, op_code> {
  public:
    string Do(string A, string B) {
      LogicBase<double, op_code> base;
      return base.Do(A, B);
    }
  };

  template<OperatorCode op_code>
  class GroupLogicBase<G_STR, op_code> {
  public:
    string Do(string A, string B) {
      StringBase<op_code> base;
      return base.Do(A, B);
    }
  };

  template<OperatorCode op_code>
  Message CalcOperation(ObjectMap &p) {
    GroupCalcBase<G_INT, op_code> int_base;
    GroupCalcBase<G_FLOAT, op_code> float_base;
    GroupCalcBase<G_STR, op_code> string_base;
    Object &A = p["first"], &B = p["second"];
    string data_A = GetObjectStuff<string>(A);
    string data_B = GetObjectStuff<string>(B);
    GroupTypeEnum group_type = GetGroupType(A, B);
    string result_str;
    switch (group_type) {
    case G_INT:result_str = int_base.Do(data_A, data_B); break;
    case G_FLOAT:result_str = float_base.Do(data_A, data_B); break;
    case G_STR:result_str = string_base.Do(data_A, data_B); break;
    default:break;
    }
    return Message(result_str);
  }

  template<OperatorCode op_code>
  Message LogicOperation(ObjectMap &p) {
    GroupLogicBase<G_INT, op_code> int_base;
    GroupLogicBase<G_FLOAT, op_code> float_base;
    GroupLogicBase<G_STR, op_code> string_base;
    Object &A = p["first"], &B = p["second"];
    string data_A = GetObjectStuff<string>(A);
    string data_B = GetObjectStuff<string>(B);
    GroupTypeEnum group_type = GetGroupType(A, B);
    string result_str;
    switch (group_type) {
    case G_INT:result_str = int_base.Do(data_A, data_B); break;
    case G_FLOAT:result_str = float_base.Do(data_A, data_B); break;
    case G_STR:result_str = string_base.Do(data_A, data_B); break;
    default:break;
    }
    return Message(result_str);
  }

  template <OperatorCode op_code,GenericTokenEnum gen_token>
  Entry LogicBinaryOperator() {
    return Entry(LogicOperation<op_code>, "first|second", gen_token);
  }

  template <OperatorCode op_code, GenericTokenEnum gen_token>
  Entry BinaryOperator() {
    return Entry(CalcOperation<op_code>, "first|second", gen_token);
  }
}
