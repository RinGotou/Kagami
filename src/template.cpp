#include "template.h"
#include <iostream>

namespace kagami {
  //Array
  shared_ptr<void> ArrayCopy(shared_ptr<void> target) {
    const auto ptr = static_pointer_cast<vector<Object>>(target);
    vector<Object> base;

    for (auto &unit : *ptr) {
      base.push_back(unit);
    }
    return make_shared<vector<Object>>(std::move(base));
  }

  //Null
  shared_ptr<void> NullCopy(shared_ptr<void> target) {
    return make_shared<int>(0);
  }

  Message ArrayConstructor(ObjectMap &p) {
    Message result;
    auto size = p["size"];
    Object initValue = Object();
    auto it = p.find("init_value");
    if (it != p.end()) initValue = it->second;

    const auto sizeValue = stoi(*static_pointer_cast<string>(size.Get()));
    vector<Object> base;

    if (sizeValue <= 0) {
      result.combo(kStrFatalError, kCodeIllegalParm, "Illegal array size.");
      return result;
    }

    const auto typeId = initValue.GetTypeId();
    const auto methods = initValue.GetMethods();
    const auto TokenTypeEnum = initValue.GetTokenType();
    shared_ptr<void> initPtr;
    base.reserve(sizeValue);

    for (auto count = 0; count < sizeValue; count++) {
      initPtr = type::GetObjectCopy(initValue);
      base.emplace_back((Object()
        .Set(initPtr, typeId)
        .SetMethods(methods)
        .SetTokenType(TokenTypeEnum).SetRo(false)));
    }

    result.SetObject(Object()
      .SetConstructorFlag()
      .Set(make_shared<vector<Object>>(base), kTypeIdArrayBase)
      .SetMethods(type::GetPlanner(kTypeIdArrayBase)->GetMethods())
      .SetRo(false)
      , "__result");
    return result;
  }
  
  Message GetElement(ObjectMap &p) {
    Message result;
    Object temp;
    size_t size;
    int count0;
    auto object = p.at(kStrObject), subscript1 = p["subscript_1"];
    const auto typeId = object.GetTypeId();

    const auto makeStrToken = [](char target)->string {
      return string().append("'").append(1, target).append("'");
    };

    if (typeId == kTypeIdRawString) {
      auto data = *static_pointer_cast<string>(object.Get());
      if (Kit::IsString(data)) {
        data = Kit::GetRawString(data);
      }
      count0 = stoi(*static_pointer_cast<string>(subscript1.Get()));
      size = data.size();
      if (count0 <= int(size - 1)) {
        result.combo(kStrRedirect, kCodeSuccess, makeStrToken(data.at(count0)));
      }
      else {
        result.combo(kStrFatalError, kCodeOverflow, "Subscript is out of range");
      }
    }
    else if (typeId == kTypeIdArrayBase) {
      count0 = stoi(*static_pointer_cast<string>(subscript1.Get()));
      size = static_pointer_cast<vector<Object>>(object.Get())->size();
      if (count0 <= int(size - 1)) {
        auto &target = static_pointer_cast<vector<Object>>(object.Get())->at(count0);
        temp.Ref(target);
        result.SetObject(temp, "__element");
      }
      else {
        result.combo(kStrFatalError, kCodeOverflow, "Subscript is out of range");
      }
    }

    return result;
  }

  Message GetSize(ObjectMap &p) {
    Message result;
    auto object = p.at(kStrObject);
    const auto typeId = object.GetTypeId();

    if (typeId == kTypeIdArrayBase) {
      result.SetDetail(to_string(static_pointer_cast<vector<Object>>(object.Get())->size()));
    }
    else if (typeId == kTypeIdRawString) {
      auto str = *static_pointer_cast<string>(object.Get());
      if (Kit::IsString(str)) str = Kit::GetRawString(str);
      result.SetDetail(to_string(str.size()));
    }

    result.SetValue(kStrRedirect);
    return result;
  }

  Message PrintRawString(ObjectMap &p) {
    Message result;
    auto object = p.at("object");
    string msg;
    auto needConvert = false;

    if (object.GetTypeId() == kTypeIdRawString) {
      auto data = *static_pointer_cast<string>(object.Get());
      if (Kit::IsString(data)) data = Kit::GetRawString(data);
      for (size_t count = 0; count < data.size(); ++count) {
        if (data.at(count) == '\\') {
          needConvert = true;
          continue;
        }
        if (needConvert) {
          msg.append(1, Kit::ConvertChar(data.at(count)));
          needConvert = false;
        }
        else {
          msg.append(1, data.at(count));
        }
      }
      std::cout << msg << std::endl;
    }
    return result;
  }


  Message PrintArray(ObjectMap &p) {
    Message result;
    Object object = p.at("object");
    ObjectMap map;
    if (object.GetTypeId() == kTypeIdArrayBase) {
      auto &base = *static_pointer_cast<vector<Object>>(object.Get());
      auto provider = entry::Order("print", kTypeIdNull, -1);
      for (auto &unit : base) {
        map.insert(pair<string, Object>("object", unit));
        result = provider.Start(map);
        map.clear();
      }
    }
    return result;
  }

  void InitPlanners() {
    using type::AddTemplate;
    using entry::AddEntry;
    AddTemplate(kTypeIdRawString, ObjectPlanner(SimpleSharedPtrCopy<string>, "size|substr|__at|__print"));
    AddTemplate(kTypeIdArrayBase, ObjectPlanner(ArrayCopy, "size|__at|__print"));
    AddTemplate(kTypeIdNull, ObjectPlanner(NullCopy, ""));


    AddEntry(Entry(PrintRawString, kCodeNormalParm, "", "__print", kTypeIdRawString, kFlagMethod));
    AddEntry(Entry(GetElement, kCodeNormalParm, "subscript_1", "__at", kTypeIdRawString, kFlagMethod));
    AddEntry(Entry(ArrayConstructor, kCodeAutoFill, "size|init_value", "array"));
    AddEntry(Entry(GetElement, kCodeNormalParm, "subscript_1", "__at", kTypeIdArrayBase, kFlagMethod));
    AddEntry(Entry(PrintArray, kCodeNormalParm, "", "__print", kTypeIdArrayBase, kFlagMethod));
    AddEntry(Entry(GetSize, kCodeNormalParm, "", "size", kTypeIdRawString, kFlagMethod));
    AddEntry(Entry(GetSize, kCodeNormalParm, "", "size", kTypeIdArrayBase, kFlagMethod));
  }


}