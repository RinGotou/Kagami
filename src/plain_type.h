#pragma once
#include "machine_kisaragi.h"

namespace kagami {
  long IntProducer(Object& obj);
  double FloatProducer(Object& obj);
  string StringProducer(Object &obj);
  bool BoolProducer(Object &obj);

  enum PlainType {
    kPlainInt, kPlainFloat, kPlainString, kPlainBool, kNotPlainType
  };

  PlainType FindTypeCode(string type_id);

  using TypeKey = pair<string, PlainType>;
  const map<string, PlainType> kTypeStore = {
    TypeKey(kTypeIdInt, kPlainInt),
    TypeKey(kTypeIdFloat, kPlainFloat),
    TypeKey(kTypeIdString, kPlainString),
    TypeKey(kTypeIdBool, kPlainBool)
  };

  const vector<GenericToken> kStringOpStore = {
    kTokenPlus, kTokenNotEqual, kTokenEquals
  };

  using ResultTraitKey = pair<PlainType, PlainType>;
  using TraitUnit = pair<ResultTraitKey, PlainType>;
  const map<ResultTraitKey, PlainType> kResultDynamicTraits = {
    TraitUnit(ResultTraitKey(kPlainInt, kPlainInt), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainString, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainBool, kPlainBool), kPlainBool),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainFloat), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainInt, kPlainBool), kPlainInt),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainInt), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainString), kPlainString),
    TraitUnit(ResultTraitKey(kPlainFloat, kPlainBool), kPlainFloat),
    TraitUnit(ResultTraitKey(kPlainString, kPlainInt), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainFloat), kPlainString),
    TraitUnit(ResultTraitKey(kPlainString, kPlainBool), kPlainString)
  };

  template <class ResultType, class Tx, class Ty, GenericToken op>
  struct BinaryOpBox {
    ResultType Do(Tx A, Ty B) {
      return Tx();
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenPlus> {
    ResultType Do(Tx A, Ty B) {
      return A + B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenMinus> {
    ResultType Do(Tx A, Ty B) {
      return A - B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenTimes> {
    ResultType Do(Tx A, Ty B) {
      return A * B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenDivide> {
    ResultType Do(Tx A, Ty B) {
      return A / B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenBitAnd> {
    ResultType Do(Tx A, Ty B) {
      return A & B;
    }
  };

  template <class ResultType, class Tx, class Ty>
  struct BinaryOpBox<ResultType, Tx, Ty, kTokenBitOr> {
    ResultType Do(Tx A, Ty B) {
      return A | B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenEquals> {
    bool Do(Tx A, Ty B) {
      return A == B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenLessOrEqual> {
    bool Do(Tx A, Ty B) {
      return A <= B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenGreaterOrEqual> {
    bool Do(Tx A, Ty B) {
      return A >= B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenNotEqual> {
    bool Do(Tx A, Ty B) {
      return A != B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenGreater> {
    bool Do(Tx A, Ty B) {
      return A > B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenLess> {
    bool Do(Tx A, Ty B) {
      return A < B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenAnd> {
    bool Do(Tx A, Ty B) {
      return A && B;
    }
  };

  template <class Tx, class Ty>
  struct BinaryOpBox<bool, Tx, Ty, kTokenOr> {
    bool Do(Tx A, Ty B) {
      return A || B;
    }
  };


#define DISPOSE_STRING_MATH_OP(OP)                 \
  template <>                                      \
  struct BinaryOpBox<string, string, string, OP> { \
    string Do(string A, string B) {                \
      return string();                             \
    }                                              \
  }                                                \

#define DISPOSE_STRING_LOGIC_OP(OP)                \
  template <>                                      \
  struct BinaryOpBox<bool, string, string, OP> {   \
    bool Do(string A, string B) {                  \
      return false;                                \
    }                                              \
  }                                                \

  DISPOSE_STRING_MATH_OP(kTokenMinus);
  DISPOSE_STRING_MATH_OP(kTokenTimes);
  DISPOSE_STRING_MATH_OP(kTokenDivide);
  DISPOSE_STRING_MATH_OP(kTokenBitAnd);
  DISPOSE_STRING_MATH_OP(kTokenBitOr);
  DISPOSE_STRING_LOGIC_OP(kTokenLessOrEqual);
  DISPOSE_STRING_LOGIC_OP(kTokenGreaterOrEqual);
  DISPOSE_STRING_LOGIC_OP(kTokenGreater);
  DISPOSE_STRING_LOGIC_OP(kTokenLess);
  DISPOSE_STRING_LOGIC_OP(kTokenAnd);
  DISPOSE_STRING_LOGIC_OP(kTokenOr);

#undef DISPOSE_STRING_MATH_OP
#undef DISPOSE_STRING_LOGIC_OP

  template <class ResultType, GenericToken op>
  using MathBox = BinaryOpBox<ResultType, ResultType, ResultType, op>;

  template <class Tx, GenericToken op>
  using LogicBox = BinaryOpBox<bool, Tx, Tx, op>;

  template <GenericToken op>
  Message BinaryOperator_Math(ObjectMap &p) {
    auto &rhs = p["right"];
    auto &lhs = p["left"];
    Object obj;

    if (!util::IsPlainType(lhs.GetTypeId()) 
      || !util::IsPlainType(rhs.GetTypeId())) {
      return Message().SetObject(obj);
    }

    auto type_rhs = FindTypeCode(rhs.GetTypeId());
    auto type_lhs = FindTypeCode(lhs.GetTypeId());
    PlainType type = kResultDynamicTraits.at(
      ResultTraitKey(type_lhs, type_rhs));
    
    if (type == kPlainString) {
      if (!find_in_vector(op, kStringOpStore)) 
        return Message().SetObject(Object());

      string result = MathBox<string, op>()
        .Do(StringProducer(lhs), StringProducer(rhs));

      obj.ManageContent(make_shared<string>(result), kTypeIdString);
    }
    else if (type == kPlainInt) {
      long result = MathBox<long, op>()
        .Do(IntProducer(lhs), IntProducer(rhs));

      obj.ManageContent(make_shared<long>(result), kTypeIdInt);
    }
    else if (type == kPlainFloat) {
      double result = MathBox<double, op>()
        .Do(FloatProducer(lhs), FloatProducer(rhs));

      obj.ManageContent(make_shared<double>(result), kTypeIdFloat);
    }
    else if (type == kPlainBool) {
      long result = MathBox<long, op>()
        .Do(IntProducer(lhs), IntProducer(rhs));
    }

    return Message().SetObject(obj);
  }

  template <GenericToken op>
  void BinaryMathOperatorGenerator() {
    management::CreateGenericInterface(
      Interface(BinaryOperator_Math<op>, "left|right", op)
    );
  }

  template <GenericToken op>
  Message BinaryOperator_Logic(ObjectMap &p) {
    auto &rhs = p["right"];
    auto &lhs = p["left"];

    if (!util::IsPlainType(lhs.GetTypeId())) {
      if (op != kTokenEquals && op != kTokenNotEqual) {
        return Message().SetObject(Object());
      }

      auto methods = management::type::GetMethods(lhs.GetTypeId());

      if (!find_in_vector(kStrCompare, methods)) {
        return Message(kCodeBadExpression, "Invalid operation", kStateError);
      }

      ObjectMap obj_map = {
        NamedObject(kStrRightHandSide, rhs),
        NamedObject(kStrObject, lhs)
      };

      auto interface = management::FindInterface(kStrCompare, lhs.GetTypeId());
      auto result = interface.Start(obj_map);
      auto obj = result.GetObj();
      bool value = obj.Cast<bool>();
      return Message().SetObject(
        op == kTokenNotEqual ? !value : value);
    }

    auto type_rhs = kTypeStore.at(rhs.GetTypeId());
    auto type_lhs = kTypeStore.at(lhs.GetTypeId());
    auto type = kResultDynamicTraits.at(
      ResultTraitKey(type_lhs, type_rhs));
    bool result;

    if (type == kPlainString) {
      if (!find_in_vector(op, kStringOpStore))
        return Message().SetObject(Object());

      result = LogicBox<string, op>()
        .Do(StringProducer(lhs), StringProducer(rhs));
    }
    else if (type == kPlainInt) {
      result = LogicBox<long, op>()
        .Do(IntProducer(lhs), IntProducer(rhs));
    }
    else if (type == kPlainFloat) {
      result = LogicBox<double, op>()
        .Do(FloatProducer(lhs), FloatProducer(rhs));
    }
    else if (type == kPlainBool) {
      result = LogicBox<bool, op>()
        .Do(BoolProducer(lhs), BoolProducer(rhs));
    }

    return Message().SetObject(result);
  }

  template <GenericToken op>
  void BinaryLogicOperatorGenerator() {
    management::CreateGenericInterface(
      Interface(BinaryOperator_Logic<op>, "left|right", op)
    );
  }
}
