#include "processor.h"

namespace kagami {
  Message Processor::Run() {
    deque<Object> retBase;
    vector<Inst>::iterator it = instBase.begin();
    Message msg;
    auto &manager = entry::GetCurrentManager();

    auto getObject = [&](Object &obj) -> Object{
      if (obj.IsRetSign()) {
        Object res = retBase.front();
        retBase.pop_front();
        return res;
      }
      if (obj.IsArgSign()) {
        return Object().Ref(*entry::FindObject(obj.GetOriginId()));
      }
      return obj;
    };

    for (; it != instBase.end(); it++) {
      ObjectMap objMap;
      auto &ent = it->first;
      auto &parms = it->second;
      size_t entParmSize = ent.GetParmSize();
      if (ent.GetEntrySign()) {
        string id = ent.GetId();
        string typeId;
        ent.NeedSpecificType() ?
          typeId = getObject(parms.back()).GetTypeId() :
          typeId = kTypeIdNull;
        ent = entry::Order(id, typeId);
        if (!ent.Good()) {
          msg.combo(kStrFatalError, kCodeIllegalCall, "Function not found - " + id);
          break;
        }
      }

      auto args = ent.GetArguments();
      auto mode = ent.GetArgumentMode();
      size_t idx = 0;
      if (mode == kCodeAutoSize) {
        const size_t parmSize = entParmSize - 1;
        while (idx < parmSize) {
          objMap.insert(NamedObject(args[idx], getObject(parms[idx])));
          ++idx;
        }
        string argGroupHead = args.back();
        size_t count = 0;
        while (idx < parms.size()) {
          objMap.insert(NamedObject(argGroupHead + to_string(count), getObject(parms[idx])));
          idx++;
          count++;
        }
        objMap.insert(NamedObject("__size", Object()
          .Manage(to_string(parms.size()))
          .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
          .SetTokenType(T_INTEGER)));
      }
      else {
        while (idx < args.size()) {
          if (idx >= parms.size() && ent.GetArgumentMode() == kCodeAutoFill) break;
          objMap.insert(NamedObject(args[idx], getObject(parms[idx])));
          ++idx;
        }
      }

      if (ent.GetFlag() == kFlagMethod) {
        objMap.insert(NamedObject(kStrObject, getObject(parms.back())));
      }

      msg = ent.Start(objMap);
      const auto code = msg.GetCode();
      const auto value = msg.GetValue();
      const auto detail = msg.GetDetail();

      if (value == kStrFatalError) break;

      if (code == kCodeObject) {
        auto object = msg.GetObj();
        retBase.emplace_back(object);
      }
      else if ((value == kStrRedirect && code == kCodeSuccess 
        || code == kCodeHeadPlaceholder)
        && ent.GetTokenEnum() != GT_DOT) {
        Object obj;
        obj.Manage(detail)
          .SetRetSign()
          .SetMethods(type::GetPlanner(kTypeIdRawString)->GetMethods())
          .SetTokenType(kagami::Kit::GetTokenType(detail));
        if (entry::IsOperatorToken(ent.GetTokenEnum()) 
          && it + 1 != instBase.end()) {
          auto token = (it + 1)->first.GetTokenEnum();
          if (entry::IsOperatorToken(token)) {
            retBase.emplace_front(obj);
          }
          else {
            retBase.emplace_back(obj);
          }
        }
        else {
          retBase.emplace_back(obj);
        }
      }
    }
    return msg;
  }

  Message Processor::Activiate(size_t mode) {
    using namespace entry;
    Message result;

    auto token = entry::GetGenericToken(mainToken.first);
    switch (mode) {
    case kModeDef:
      if (token == GT_WHILE || token == GT_IF) {
        return Message(kStrRedirect, kCodeHeadPlaceholder, kStrTrue);
      }
      else if (token != GT_END) {
        return Message(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
      }
      break;
    case kModeNextCondition:
      if (token == GT_IF || token == GT_WHILE) {
        return Message(kStrRedirect, kCodeHeadPlaceholder, kStrTrue);
      }
      else if (token != GT_ELSE && token != GT_END && token != GT_ELIF) {
        return Message(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
      }
      break;
    case kModeCycleJump:
      if (token != GT_END && token != GT_IF && token != GT_WHILE) {
        return Message(kStrRedirect, kCodeSuccess, kStrPlaceHolder);
      }
    default:break;
    }

    return Run();
  }
}