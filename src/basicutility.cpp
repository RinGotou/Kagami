#include "basicutility.h"

namespace Entry {
  using namespace Suzu;
  deque<MemoryProvider> childbase;
}

namespace Suzu {

  Message CommaExpression(vector<string> &res) {
    Message result;
    if (res.empty()) result.SetCode(kCodeRedirect).SetValue(kStrEmpty);
    else result.SetCode(kCodeRedirect).SetValue(res.back());
    return result;
  }

  Message MemoryQuery(vector<string> &res) {
    using namespace Entry;
    Message result;
    string temp;
    size_t begin = childbase.size() - 1;
    size_t i = 0;

    if (childbase.empty()) {
      Tracking::log(result.SetValue(kStrWarning)
        .SetCode(kCodeIllegalArgs)
        .SetDetail("MemoryQuery() 1"));
    }
    else {
      if (childbase.size() == 1 || res[1] == kArgOnce) {
        temp = childbase.back().query(res[0]);
      }
      else if (childbase.size() > 1 && res[1] != kArgOnce) {
        for (i = begin; i >= 0; --i) {
          temp = childbase[i].query(res[0]);
          if (temp != kStrNull) break;
        }
      }

      if (temp != kStrNull) {
        result.SetCode(kCodeSuccess).SetValue(kStrSuccess).SetDetail(temp);
      }
      else {
        Tracking::log(result.SetCode(kCodeIllegalCall)
          .SetValue(kStrFatalError)
          .SetDetail("MemoryQuery() 2"));
      }
    }

    return result;
  }

  Message LogPrint(vector<string> &res) {
    using namespace Tracking;
    Message result;
    ofstream ofs("event.log", std::ios::trunc);
    ofs << res.at(0) << '\n';
    ofs.close();
    return result;
  }

  Message TwoFactorCalc(vector<string> &res) {
    using Entry::childbase;
    int intA = 0, intB = 0;
    double doubleA = 0.0, doubleB = 0.0;
    enum { EnumDouble, EnumInt, EnumNull }type = EnumNull;
    Message result;
    auto CheckingOr = [&](regex pat) -> bool {
      return (regex_match(res.at(0), pat) || regex_match(res.at(1), pat));
    };
    auto CheckingAnd = [&](regex pat) -> bool {
      return (regex_match(res.at(0), pat) && regex_match(res.at(1), pat));
    };

    if (res.size() < 3) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Calculating() 1");
      return result;
    }
    //may not need at there?
    //Setup filter on query may be better
    if (!regex_match(res.at(2), kPatternSymbol)
      ||!regex_match(res.at(0),kPatternNumber)
      || !regex_match(res.at(1), kPatternNumber)) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Calculating() 2");
      return result;
    }

    //start converting
    //vector data format:number number operator
    else if (CheckingAnd(kPatternFunction)) {

    }
    else {
      if (CheckingOr(kPatternDouble)) {
        type = EnumDouble;
      }
      else if (CheckingAnd(kPatternInteger)) {
        type = EnumInt;
      }
      //TODO:dispose and return
    }

    switch (type) {
    case EnumInt: 
      intA = stoi(res.at(0));
      intB = stoi(res.at(1));
      result.SetValue(to_string(Util().Calc(intA, intB, res.at(2))));
      break;
    case EnumDouble:
      doubleA = stod(res.at(0));
      doubleB = stod(res.at(1));
      result.SetValue(to_string(Util().Calc(intA, intB, res.at(2))));
      break;
    default:
      result.combo(kStrFatalError, kCodeIllegalArgs, "Calculating() 4");
      break;
    }

    return result;
  }

  void TotalInjection() {
    using namespace Entry;
    //set root memory provider
    childbase.push_back(MemoryProvider());
    childbase.back().SetParent(&(childbase.back()));
    //inject basic Entry provider
    Inject(EntryProvider("commaexp", CommaExpression, kFlagAutoSize));
    Inject(EntryProvider("memquery", MemoryQuery, 2, kFlagCoreEntry));
    Inject(EntryProvider("twofacto", TwoFactorCalc, 3, kFlagCoreEntry));
    Inject(EntryProvider("log", LogPrint, 2));
  }
}