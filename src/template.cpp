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
#include "template.h"
#include <iostream>

namespace kagami {
  namespace type {
    using TagStore = pair<string, string>;
    map<string,string>& GetTagMap() {
      static map<string, string> base;
      return base;
    }

    string FindGoods(string name) {
      auto &base = GetTagMap();
      auto it = base.find(name);
      if (it != base.end()) return it->second;
      else return kStrEmpty;
    }
  }
  //FileStream

  //WideString


  //RawString
  //shared_ptr<void> StringCopy(shared_ptr<void> target) {
  //  auto temp(*static_pointer_cast<string>(target));
  //  return make_shared<string>(temp);
  //}

  //Array
  shared_ptr<void> ArrayCopy(shared_ptr<void> target) {
    const auto ptr = static_pointer_cast<vector<Object>>(target);
    vector<Object> base;

    for (auto &unit : *ptr) {
      base.push_back(unit);
    }
    return make_shared<vector<Object>>(std::move(base));
  }

  //Cube

  //Regex

  //Null
  shared_ptr<void> NullCopy(shared_ptr<void> target) {
    return make_shared<int>(0);
  }

  Message ArrayConstructor(ObjectMap &p) {
    Message result;
    Kit kit;
    auto size = p.at("size"), initValue = p.at("init_value");
    const auto sizeValue = stoi(*static_pointer_cast<string>(size.Get()));
    vector<Object> base;

    //error:wrong size
    if (sizeValue <= 0) {
      result.combo(kStrFatalError, kCodeIllegalArgs, "Illegal array size.");
      return result;
    }

    auto attribute = initValue.GetTag();
    attribute.ro = false;
    const auto attrStr = kit.BuildAttrStr(attribute);
    const auto typeId = initValue.GetTypeId();
    base.reserve(sizeValue);

    for (auto count = 0; count < sizeValue; count++) {
      auto initPtr = type::GetObjectCopy(initValue);
      base.emplace_back(std::move(Object().Set(initPtr, typeId, attrStr)));
    }

    attribute.methods = type::GetTemplate(kTypeIdArrayBase)->GetMethods();
    attribute.ro = false;
    const auto arrayTag = kit.BuildAttrStr(attribute);
    result.SetObject(Object().Set(make_shared<vector<Object>>(base), kTypeIdArrayBase, arrayTag), "__result");
    return result;
  }
  
  Message GetElement(ObjectMap &p) {
    Kit kit;
    Message result;
    Object temp;
    size_t size;
    int count0;
    auto object = p.at(kStrObject), subscript1 = p.at("subscript_1");
    const auto typeId = object.GetTypeId();

    const auto makeStrToken = [](char target)->string {
      return string().append("'").append(1, target).append("'");
    };

    if (typeId == kTypeIdRawString) {
      auto data = *static_pointer_cast<string>(object.Get());
      if (kit.IsString(data)) {
        data = kit.GetRawString(data);
      }
      count0 = stoi(*static_pointer_cast<string>(subscript1.Get()));
      size = data.size();
      if (count0 <= int(size - 1)) {
        result.combo(kStrRedirect, kCodeSuccess, makeStrToken(data.at(count0)));
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
      if (Kit().IsString(str)) str = Kit().GetRawString(str);
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
      if (Kit().IsString(data)) data = Kit().GetRawString(data);
      for (size_t count = 0; count < data.size(); ++count) {
        if (data.at(count) == '\\') {
          needConvert = true;
          continue;
        }
        if (needConvert) {
          msg.append(1, Kit().ConvertChar(data.at(count)));
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

  void InitTemplates() {
    using type::AddTemplate;
    auto &base = type::GetTagMap();
    Attribute attr;
    Kit kit;
    auto Build = [&kit](Attribute target) {return kit.BuildAttrStr(target); };
    AddTemplate(kTypeIdRawString, ObjTemplate(SimpleSharedPtrCopy<string>, "size|substr|at|__print"));
    AddTemplate(kTypeIdArrayBase, ObjTemplate(ArrayCopy, "size|at|__print"));
    AddTemplate(kTypeIdNull, ObjTemplate(NullCopy, ""));
    base.insert(type::TagStore(kTypeIdRawString, Build(attr.Set("size|substr|at|__print",false))));
  }

  void InitMethods() {
    using namespace entry;
    ActivityTemplate temp;
    //constructor
    Inject(EntryProvider(temp.Set("array", ArrayConstructor, kFlagNormalEntry, kCodeAutoFill, "size|init_value")));
    //methods
    Inject(EntryProvider(temp.Set("at", GetElement, kFlagMethod, kCodeNormalArgs, "subscript_1", kTypeIdRawString)));
    Inject(EntryProvider(temp.Set("at", GetElement, kFlagMethod, kCodeNormalArgs, "subscript_1", kTypeIdArrayBase)));
    //Inject(EntryProvider(temp.Set("at", GetElement2Dimension, kFlagMethod, kCodeNormalArgs, "subscript_1|subscript_2", kTypeIdCubeBase)));
    Inject(EntryProvider(temp.Set("__print", PrintRawString, kFlagMethod, kCodeNormalArgs, "", kTypeIdRawString)));
    Inject(EntryProvider(temp.Set("__print", PrintArray, kFlagMethod, kCodeNormalArgs, "", kTypeIdArrayBase)));
    Inject(EntryProvider(temp.Set("size", GetSize, kFlagMethod, kCodeNormalArgs, "", kTypeIdRawString)));
    Inject(EntryProvider(temp.Set("size", GetSize, kFlagMethod, kCodeNormalArgs, "", kTypeIdArrayBase)));
  }


}