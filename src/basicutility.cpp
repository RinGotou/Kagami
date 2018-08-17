#include "parser.h"

namespace kagami {
  namespace type {
    map <string, ObjectPlanner> &GetPlannerBase() {
      static map<string, ObjectPlanner> base;
      return base;
    }

    shared_ptr<void> GetObjectCopy(Object &object) {
      if (object.ConstructorFlag()) {
        return object.Get();
      }

      shared_ptr<void> result = nullptr;
      const auto option = object.GetTypeId();
      const auto it = GetPlannerBase().find(option);

      if (it != GetPlannerBase().end()) {
        result = it->second.CreateObjectCopy(object.Get());
      }
      return result;
    }

    ObjectPlanner *GetPlanner(const string name) {
      ObjectPlanner *result = nullptr;
      const auto it       = GetPlannerBase().find(name);

      if (it != GetPlannerBase().end()) {
        result = &(it->second);
      }
      return result;
    }

    void AddTemplate(string name, ObjectPlanner temp) {
      GetPlannerBase().insert(pair<string, ObjectPlanner>(name, temp));
    }

    void DisposeTemplate(const string name) {
      const auto it = GetPlannerBase().find(name);
      if (it != GetPlannerBase().end()) GetPlannerBase().erase(it);
    }
  }

  Message WriteLog(ObjectMap &p) {
    Kit kit;
    Message result;
    auto data = p["data"];
    ofstream ofs("script.log", std::ios::out | std::ios::app);

    if (data.GetTypeId() == kTypeIdRawString) {
      const auto ptr = static_pointer_cast<string>(data.Get());
      if (kit.IsString(*ptr)) {
        ofs << ptr->substr(1, ptr->size() - 2) << "\n";
      }
      else {
        ofs << *ptr << "\n";
      }
    }
    else {
      //TODO:query
    }
    ofs.close();
    return result;
  }

  Message BinaryOperands(ObjectMap &p) {
    using entry::OperatorCode;

    Kit kit;
    Message result(kStrRedirect, kCodeSuccess, "0");
    string temp, dataOP;
    auto first = p["first"], second = p["second"], op = p[kStrOperator];
    auto tempresult = false;
    enum { enum_int, enum_double, enum_str, enum_null } enumtype = enum_null;

    if (op.Get() != nullptr) dataOP = *static_pointer_cast<string>(op.Get());
    auto opCode = entry::GetOperatorCode(dataOP);

    if (first.GetTypeId() == kTypeIdRawString && second.GetTypeId() == kTypeIdRawString) {
      auto dataA = *static_pointer_cast<string>(first.Get());
      auto dataB = *static_pointer_cast<string>(second.Get());
      const auto datatypeA = first.GetTokenType();
      const auto datatypeB = second.GetTokenType();
      if (datatypeA == T_DOUBLE || datatypeB == T_DOUBLE) enumtype = enum_double;
      if (datatypeA == T_INTEGER && datatypeB == T_INTEGER) enumtype = enum_int;
      if (kit.IsString(dataA) || kit.IsString(dataB)) enumtype = enum_str;

      if (enumtype == enum_int || enumtype == enum_double) {
        switch (opCode) {
        case OperatorCode::ADD:
        case OperatorCode::SUB:
        case OperatorCode::MUL:
        case OperatorCode::DIV:
          switch (enumtype) {
          case enum_int:   temp = to_string(kit.Calc(stoi(dataA), stoi(dataB), dataOP)); break;
          case enum_double:temp = to_string(kit.Calc(stod(dataA), stod(dataB), dataOP)); break;
          default:;
          }
          break;
        case OperatorCode::IS:
        case OperatorCode::MORE_OR_EQUAL:
        case OperatorCode::LESS_OR_EQUAL:
        case OperatorCode::NOT_EQUAL:
        case OperatorCode::MORE:
        case OperatorCode::LESS:
          switch (enumtype) {
          case enum_int:   tempresult = kit.Logic(stoi(dataA), stoi(dataB), dataOP); break;
          case enum_double:tempresult = kit.Logic(stod(dataA), stod(dataB), dataOP); break;
          default:;
          }
          tempresult ? temp = kStrTrue : temp = kStrFalse;
          break;
        default:break;
        }
      }
      else if (enumtype == enum_str) {
        switch (opCode) {
        case OperatorCode::ADD:
          if (dataA.back() == '\'') {
            temp  = dataA.substr(0, dataA.size() - 1);
            dataA = temp;
          }
          if (dataB.front() == '\'') {
            temp  = dataB.substr(1, dataB.size() - 1);
            dataB = temp;
          }
          if (dataB.back() != '\'') {
            dataB.append(1, '\'');
          }
          temp = dataA + dataB;
          break;
        case OperatorCode::NOT_EQUAL:
        case OperatorCode::EQUAL:
          tempresult = kit.Logic(dataA, dataB, dataOP);
          tempresult ? temp = kStrTrue : temp = kStrFalse;
          break;
        case OperatorCode::MORE_OR_EQUAL:
        case OperatorCode::LESS_OR_EQUAL:
          //TODO:add in Kit::Logic()
          break;
        default:break;
        }
      }
      else {
        //TODO:other type
      }
    }
    result.SetDetail(temp);

    return result;
  }

  Message ConditionRoot(ObjectMap &p) {
    return Message(*static_pointer_cast<string>(p["state"].Get()), kCodeConditionRoot, kStrEmpty);
  }

  Message ConditionBranch(ObjectMap &p) {
    return Message(*static_pointer_cast<string>(p["state"].Get()), kCodeConditionBranch, kStrEmpty);
  }

  Message ConditionLeaf(ObjectMap &p) {
    return Message(kStrTrue, kCodeConditionLeaf, kStrEmpty);
  }

  Message WhileCycle(ObjectMap &p) {
    return Message(*static_pointer_cast<string>(p["state"].Get()), kCodeHeadSign, kStrEmpty);
  }

  Message TailSign(ObjectMap &p) {
    return Message(kStrEmpty, kCodeTailSign, kStrEmpty);
  }

  Message LeftSelfIncreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if(object.GetTypeId() == kTypeIdRawString) {
      const auto origin = *static_pointer_cast<string>(object.Get());
      if (object.GetTokenType() == T_INTEGER) {
        auto data = stoi(origin);
        ++data;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
      else if (object.GetTokenType() == T_DOUBLE) {
        auto data = stod(origin);
        data += 1.0f;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message LeftSelfDecreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      const auto origin = *static_pointer_cast<string>(object.Get());
      if (object.GetTokenType() == T_INTEGER) {
        auto data = stoi(origin);
        --data;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
      else if (object.GetTokenType() == T_DOUBLE) {
        auto data = stod(origin);
        data -= 1.0f;
        result = to_string(data);
        object.Set(make_shared<string>(result), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message RightSelfIncreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      auto origin = *static_pointer_cast<string>(object.Get());
      result = origin;
      if (object.GetTokenType() == T_INTEGER) {
        auto data = stoi(origin);
        ++data;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
      else if (object.GetTokenType() == T_DOUBLE) {
        auto data = stod(origin);
        data += 1.0f;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }

  Message RightSelfDecreament(ObjectMap &p) {
    auto object = p["object"];
    string result;

    if (object.GetTypeId() == kTypeIdRawString) {
      auto origin = *static_pointer_cast<string>(object.Get());
      result = origin;
      if (object.GetTokenType() == T_INTEGER) {
        auto data = stoi(origin);
        --data;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
      else if (object.GetTokenType() == T_DOUBLE) {
        auto data = stod(origin);
        data -= 1.0f;
        object.Set(make_shared<string>(to_string(data)), kTypeIdRawString);
      }
    }

    return Message(kStrRedirect, kCodeSuccess, result);
  }



  Message Set(ObjectMap &p) {
    Message msg;
    Object object = p["object"], source = p["source"];
    auto copy = type::GetObjectCopy(source);

    if (object.IsRo()) {
      msg.combo(kStrFatalError, kCodeIllegalCall, "Object is read-only.");
    }
    else {
      object.Set(copy, source.GetTypeId())
        .SetMethods(source.GetMethods())
        .SetTokenType(source.GetTokenType());
    }
    
    return msg;
  }

  Message Bind(ObjectMap &p) {
    Message msg;
    Object object = p["object"], source = p["source"];
    auto id = *static_pointer_cast<string>(object.Get());
    auto ptr = entry::FindObject(id);
    auto copy = type::GetObjectCopy(source);


    Object base;
    base.Set(copy, source.GetTypeId())
      .SetMethods(source.GetMethods())
      .SetTokenType(source.GetTokenType())
      .SetRo(false);
    auto result = entry::CreateObject(id, base);
    if (result == nullptr) {
      msg.combo(kStrFatalError, kCodeIllegalCall, "Object creation failed.");
    }
    
    return msg;
  }

  Message Print(ObjectMap &p) {
    Kit kit;
    Message result;
    auto object = p["object"];

    if (kit.FindInStringGroup("__print", object.GetMethods())) {
      auto provider = entry::Order("__print", object.GetTypeId(), -1);
      if (provider.Good()) {
        result = provider.Start(p);
      }
      else {
        std::cout << "You can't print this object." << std::endl;
      }
    } 
    else {
      std::cout << "You can't print this object." << std::endl;
    }

    return result;
  }

  Message TimeReport(ObjectMap &p) {
    auto now = time(nullptr);
#if defined(_WIN32) && defined(_MSC_VER)
    char nowTime[30] = { ' ' };
    ctime_s(nowTime, sizeof(nowTime), &now);
    return Message(kStrRedirect, kCodeSuccess, "'" + string(nowTime) + "'");
#else
    string TimeData(ctime(&now));
    return Message(kStrRedirect, kCodeSuccess, "'" + TimeData + "'");
#endif
  }

  Message Quit(ObjectMap &p) {
    Message result(kStrEmpty, kCodeQuit, kStrEmpty);
    return result;
  }

  void LoadGenericProvider() {
    using namespace entry;

    LoadGenProvider(GT_END, Entry(nullptr, "", GT_END));
    LoadGenProvider(GT_ELSE, Entry(nullptr, "", GT_ELSE));
    LoadGenProvider(GT_IF, Entry(ConditionRoot, "state", GT_IF));
    LoadGenProvider(GT_SET, Entry(Set, "object|source", GT_SET));
    LoadGenProvider(GT_WHILE, Entry(WhileCycle, "state", GT_WHILE));
    LoadGenProvider(GT_BIND, Entry(Bind, "object|source", GT_BIND));
    LoadGenProvider(GT_ELIF, Entry(ConditionBranch, "state", GT_ELIF));

    //LoadGenProvider(GT_BINOP, Entry(kStrBinOp, BinaryOperands, kFlagOperatorEntry, kCodeNormalParm, "first|second"));
    //LoadGenProvider(GT_LSELF_INC, Entry(kStrLeftSelfInc, LeftSelfIncreament, kFlagNormalEntry, kCodeNormalParm, "object"));
    //LoadGenProvider(GT_LSELF_DEC, Entry(kStrLeftSelfDec, LeftSelfDecreament, kFlagNormalEntry, kCodeNormalParm, "object"));
    //LoadGenProvider(GT_RSELF_INC, Entry(kStrRightSelfInc, RightSelfIncreament, kFlagNormalEntry, kCodeNormalParm, "object"));
    //LoadGenProvider(GT_RSELF_DEC, Entry(kStrLeftSelfDec, LeftSelfDecreament, kFlagNormalEntry, kCodeNormalParm, "object"));
  }

  void Activiate() {
    using namespace entry;
    LoadGenericProvider();
    InitPlanners();

    Inject(Entry(Print, kCodeNormalParm, "object", "print"));
    Inject(Entry(TimeReport, kCodeNormalParm, "", "time"));
    Inject(Entry(Quit, kCodeNormalParm, "", "quit"));
    //Inject(Entry("log", WriteLog, kFlagNormalEntry, kCodeNormalParm, "data"));
  }
}
