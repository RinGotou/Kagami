#include "plain_type.h"

namespace kagami {
  long IntProducer(Object &obj) {
    auto type = FindTypeCode(obj.GetTypeId());
    long result = 0;
    switch (type) {
    case kPlainInt:result = obj.Cast<long>(); break;
    case kPlainFloat:result = static_cast<long>(obj.Cast<double>()); break;
    case kPlainBool:result = obj.Cast<bool>() ? 1 : 0; break;
    default:break;
    }

    return result;
  }

  double FloatProducer(Object &obj) {
    auto type = FindTypeCode(obj.GetTypeId());
    double result = 0;
    switch (type) {
    case kPlainFloat:result = obj.Cast<double>(); break;
    case kPlainInt:result = static_cast<double>(obj.Cast<long>()); break;
    case kPlainBool:result = obj.Cast<bool>() ? 1.0 : 0.0; break;
    default:break;
    }

    return result;
  }

  string StringProducer(Object &obj) {
    auto type = FindTypeCode(obj.GetTypeId());
    string result;
    switch (type) {
    case kPlainInt:result = to_string(obj.Cast<long>()); break;
    case kPlainFloat:result = to_string(obj.Cast<double>()); break;
    case kPlainBool:result = obj.Cast<bool>() ? kStrTrue : kStrFalse; break;
    case kPlainString:result = obj.Cast<string>(); break;
    default:break;
    }

    return result;
  }

  PlainType FindTypeCode(string type_id) {
    auto it = kTypeStore.find(type_id);
    return it != kTypeStore.end() ? it->second : kNotPlainType;
  }

  bool BoolProducer(Object &obj) {
    auto it = kTypeStore.find(obj.GetTypeId());
    auto type = it != kTypeStore.end() ? it->second : kNotPlainType;
    bool result = false;
    if (type == kPlainInt) {
      long value = obj.Cast<long>();
      if (value > 0) result = true;
    }
    else if (type == kPlainFloat) {
      double value = obj.Cast<double>();
      if (value > 0.0) result = true;
    }
    else if (type == kPlainBool) {
      result = obj.Cast<bool>();
    }
    else if (type == kPlainString) {
      string &value = obj.Cast<string>();
      result = !value.empty();
    }

    return result;
  }

  Message LogicNot(ObjectMap &p) {
    EXPECT_TYPE(p, kStrRightHandSide, kTypeIdBool);
    bool value = !p[kStrRightHandSide].Cast<bool>();
    return Message().SetObject(value);
  }

  void InitPlainTypeComponents() {
    using management::type::NewTypeSetup;

    NewTypeSetup(kTypeIdInt, SimpleSharedPtrCopy<long>);
    NewTypeSetup(kTypeIdFloat, SimpleSharedPtrCopy<double>);
    NewTypeSetup(kTypeIdBool, SimpleSharedPtrCopy<bool>);
    NewTypeSetup(kTypeIdNull, FakeCopy<void>);
    //NewTypeSetup(kTypeIdByte, SimpleSharedPtrCopy<Byte>);

    BinaryMathOperatorGenerator<kTokenPlus>();
    BinaryMathOperatorGenerator<kTokenMinus>();
    BinaryMathOperatorGenerator<kTokenTimes>();
    BinaryMathOperatorGenerator<kTokenDivide>();
    BinaryLogicOperatorGenerator<kTokenEquals>();
    BinaryLogicOperatorGenerator<kTokenNotEqual>();
    BinaryLogicOperatorGenerator<kTokenLessOrEqual>();
    BinaryLogicOperatorGenerator<kTokenGreaterOrEqual>();
    BinaryLogicOperatorGenerator<kTokenGreater>();
    BinaryLogicOperatorGenerator<kTokenLess>();
    BinaryLogicOperatorGenerator<kTokenAnd>();
    BinaryLogicOperatorGenerator<kTokenOr>();

    EXPORT_CONSTANT(kTypeIdInt);
    EXPORT_CONSTANT(kTypeIdFloat);
    EXPORT_CONSTANT(kTypeIdBool);
    EXPORT_CONSTANT(kTypeIdNull);
  }
}