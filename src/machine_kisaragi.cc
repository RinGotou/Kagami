#include "machine_kisaragi.h"

namespace kagami {
  string ParseString(const string &src) {
    string result = src;
    if (util::IsString(result)) result = util::GetRawString(result);
    return result;
  }

  Object Machine::FetchInterfaceObject(string id, string domain) {
    Object obj;
    auto interface = management::FindInterface(id, domain);
    if (interface.Good()) {
      obj.ManageContent(make_shared<Interface>(interface), kTypeIdFunction);
    }
    return obj;
  }

  Object Machine::FetchObject(Argument &arg, bool checking) {
    ObjectPointer ptr = nullptr;
    string domain_type_id = kTypeIdNull;
    Object obj;
    auto &worker = worker_stack_.top();
    auto &return_stack = worker_stack_.top().return_stack;

    auto fetching = [&](ArgumentType type, bool is_domain)->bool {
      switch (type) {
      case kArgumentNormal:
        obj.ManageContent(make_shared<string>(arg.data), kTypeIdRawString);
        break;

      case kArgumentObjectPool:
        ptr = obj_stack_.Find(is_domain ? arg.domain.data : arg.data);
        if (ptr != nullptr) {
          if (is_domain) {
            domain_type_id = ptr->GetTypeId();
          }
          else {
            obj.CreateRef(*ptr);
          }
        }
        else {
          obj = is_domain ?
            FetchInterfaceObject(arg.domain.data, kTypeIdNull) :
            FetchInterfaceObject(arg.data, domain_type_id);

          if (obj.Get() == nullptr) {
            worker.MakeError("Object is not found."
              + (is_domain ? arg.domain.data : arg.data));
            return false;
          }
        }
        break;

      case kArgumentReturningStack:
        if (!return_stack.empty()) {
          if (is_domain) {
            domain_type_id = return_stack.top().GetTypeId();
          }
          else {
            obj = return_stack.top();
            if (!checking) return_stack.pop();
          }
        }
        else {
          worker.MakeError("Can't get object from stack.");
          return false;
        }

        break;

      default:
        break;
      }

      return true;
    };

    if (!fetching(arg.domain.type, true)) return Object();
    if (!fetching(arg.type, false)) return Object();
    return obj;
  }

  void Machine::SetSegmentInfo(ArgumentList args) {
    MachineWorker &worker = worker_stack_.top();
    worker.origin_idx = stoul(args[0].data);
    worker.last_command = static_cast<GenericToken>(stol(args[1].data));
  }

  void Machine::CommandIfOrWhile(GenericToken token, ArgumentList args) {
    auto &worker = worker_stack_.top();
    if (args.size() == 1) {
      Object obj = FetchObject(args[0]);
      bool state = false;

      if (obj.GetTypeId() == kTypeIdRawString) {
        string state_str = obj.Cast<string>();

        if (state_str == kStrTrue) {
          state = true;
        }
        else if (state_str == kStrFalse) {
          state = false;
        }
        else {
          auto type = util::GetTokenType(state_str, true);

          switch (type) {
          case kTokenTypeInt:
            state = (stol(state_str) != 0);
            break;
          case kTokenTypeString:
            state = (ParseString(state_str).size() > 0);

            break;
          default:
            break;
          }
        }
      }
      else if (obj.GetTypeId() != kTypeIdNull) {
        state = true;
      }

      if (token == kTokenIf) {
        worker.SwitchToMode(state ? kModeCondition : kModeNextCondition);
        worker.condition_stack.push(state);
      }
      else if (token == kTokenWhile) {
        if (worker.loop_head.empty()) {
          obj_stack_.Push();
        }
        else if (worker.loop_head.top() != worker.idx - 1) {
          obj_stack_.Push();
        }

        if (worker.loop_head.empty() || worker.loop_head.top() != worker.idx - 1) {
          worker.loop_head.push(worker.idx - 1);
        }

        if (state) {
          worker.SwitchToMode(kModeCycle);
        }
        else {
          worker.SwitchToMode(kModeCycleJump);
          if (worker.loop_head.size() == worker.loop_tail.size()) {
            if (!worker.loop_tail.empty()) {
              worker.idx = worker.loop_tail.top();
            }
          }
        }
      }
      else if (token == kTokenElif) {
        if (!worker.condition_stack.empty()) {
          if (worker.condition_stack.top() == false && worker.mode == kModeNextCondition) {
            worker.mode = kModeCondition;
            worker.condition_stack.top() = true;
          }
        }
        else {
          worker.MakeError("Unexpected Elif.");
          return;
        }
      }
    }
    else {
      worker.MakeError("Too many arguments.");
      return;
    }
  }

  void Machine::CommandElse(ArgumentList args) {
    auto &worker = worker_stack_.top();
    if (!worker.condition_stack.empty()) {
      if (worker.condition_stack.top() == true) {
        switch (worker.mode) {
        case kModeCondition:
        case kModeNextCondition:
          worker.mode = kModeNextCondition;
          break;
        case kModeCase:
        case kModeCaseJump:
          worker.mode = kModeCaseJump;
          break;
        default:
          break;
        }
      }
      else {
        worker.condition_stack.top() = true;
        switch (worker.mode) {
        case kModeNextCondition:
          worker.mode = kModeCondition;
          break;
        case kModeCaseJump:
          worker.mode = kModeCase;
          break;
        default:
          break;
        }
      }
    }
    else {
      worker.MakeError("Unexpected Else.");
      return;
    }
  }

  void Machine::CommandConditionEnd() {
    auto &worker = worker_stack_.top();
    worker.condition_stack.pop();
    worker.GoLastMode();
    obj_stack_.Pop();
  }

  void Machine::CommandLoopEnd() {
    auto &worker = worker_stack_.top();
    if (worker.mode == kModeCycle) {
      if (worker.loop_tail.empty() || worker.loop_tail.top() != worker.idx - 1) {
        worker.loop_tail.push(worker.idx - 1);
      }
      worker.idx = worker.loop_head.top();
      obj_stack_.GetCurrent().clear();
    }
    else if (worker.mode == kModeCycleJump) {
      if (worker.activated_continue) {
        if (worker.loop_tail.empty() || worker.loop_tail.top() != worker.idx - 1) {
          worker.loop_tail.push(worker.idx - 1);
        }
        worker.idx = worker.loop_head.top();
        worker.mode = kModeCycle;
        worker.activated_continue = false;
        obj_stack_.GetCurrent().clear();
      }
      else {
        if (worker.activated_break) worker.activated_break = false;
        worker.GoLastMode();
        if (worker.loop_head.size() == worker.loop_tail.size()) {
          worker.loop_head.pop();
          worker.loop_tail.pop();
        }
        else {
          worker.loop_head.pop();
        }
        obj_stack_.Pop();
      }
    }
  }

  void Machine::CommandReturn(ArgumentList args) {
    if (worker_stack_.size() <= 1) {
      trace::AddEvent(Message(kCodeBadExpression, "Unexpected return.", kStateError));
    }
    else if (args.size() == 1) {
      Object src_obj = FetchObject(args[0]);
      Object ret_obj(management::type::GetObjectCopy(src_obj), src_obj.GetTypeId());
      worker_stack_.pop();
      ir_stack_.pop_back();
      obj_stack_.Pop();
      worker_stack_.top().return_stack.push(ret_obj);
    }
    else if (args.size() == 0) {
      worker_stack_.pop();
      ir_stack_.pop_back();
      obj_stack_.Pop();
      worker_stack_.top().return_stack.push(Object());
    }
    //TODO:Multiple arg
  }

  void Machine::MachineCommands(GenericToken token, ArgumentList args) {
    switch (token) {
    case kTokenSegment:
      SetSegmentInfo(args);
      break;

    default:
      break;
    }
  }

  void Machine::GenerateArgs(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
    switch (interface.GetArgumentMode()) {
    case kCodeNormalParam:
      Generate_Normal(interface, args, obj_map);
      break;
    case kCodeAutoSize:
      Generate_AutoSize(interface, args, obj_map);
      break;
    case kCodeAutoFill:
      Generate_AutoFill(interface, args, obj_map);
      break;
    default:
      break;
    }
  }

  void Machine::Generate_Normal(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
    auto &worker = worker_stack_.top();
    auto params = interface.GetParameters();

    if (args.size() > params.size()) {
      worker.MakeError("Too many arguments");
      return;
    }

    if (args.size() < params.size()) {
      worker.MakeError("Required argument count is " +
        to_string(params.size()) +
        ", but provided argument count is " +
        to_string(args.size()) + ".");
      return;
    }

    for (auto it = params.rbegin(); it != params.rend(); ++it) {
      obj_map.insert(NamedObject(*it, FetchObject(args.back())));
      args.pop_back();
    }
  }

  void Machine::Generate_AutoSize(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
    auto &worker = worker_stack_.top();
    auto params = interface.GetParameters();
    list<Object> temp_list;
    shared_ptr<ObjectArray> va_base(new ObjectArray());

    if (args.size() < params.size()) {
      worker.MakeError("Too few arguments.");
      return;
    }

    while (args.size() >= params.size() - 1 && !args.empty()) {
      temp_list.emplace_front(FetchObject(args.back()));
      args.pop_back();
    }

    if (!temp_list.empty()) {
      for (auto it = temp_list.begin(); it != temp_list.end(); ++it) {
        va_base->emplace_back(*it);
      }
    }

    temp_list.clear();
    
    auto it = ++params.rbegin();

    if (!args.empty()) {
      for (; it != params.rend(); ++it) {
        obj_map.insert(NamedObject(*it, FetchObject(args.back())));
        args.pop_back();
      }
    }
  }

  void Machine::Generate_AutoFill(Interface &interface, ArgumentList args, ObjectMap &obj_map) {
    auto &worker = worker_stack_.top();
    auto params = interface.GetParameters();
    size_t min_size = interface.GetMinArgSize();

    if (args.size() > params.size()) {
      worker.MakeError("Too many arguments");
      return;
    }

    if (args.size() < min_size) {
      worker.MakeError("Required minimum argument count is" +
        to_string(min_size) +
        ", but provided argument count is" +
        to_string(args.size()));
      return;
    }

    while (args.size() != params.size()) {
      obj_map.insert(NamedObject(params.back(), Object()));
      params.pop_back();
    }

    for (auto it = params.rbegin(); it != params.rend(); ++it) {
      obj_map.insert(NamedObject(*it, FetchObject(args.back())));
      args.pop_back();
    }
  }

  Message Machine::Run() {
    if (ir_stack_.empty()) return Message();
    StateLevel level = kStateNormal;
    StateCode code = kCodeSuccess;
    string detail;
    string type_id = kTypeIdNull;
    Message msg;
    KIR &ir = *ir_stack_.back();
    Interface interface;
    ObjectMap obj_map;

    worker_stack_.push(MachineWorker());
    obj_stack_.Push();

    MachineWorker &worker = worker_stack_.top();
    size_t size = ir.size();

    while (worker.idx < size) {
      Command &command = ir[worker.idx];

      if (worker.NeedSkipping()) {

      }

      if (command.first.type == kRequestCommand 
        && !util::IsOperator(command.first.head_command)) {
        MachineCommands(command.first.head_command, command.second);

        if (worker.deliver) {
          msg = worker.GetMsg();
          worker.msg.Clear();
        }

        if (worker.error) break;

        continue;
      }
      
      if (command.first.type == kRequestCommand) {
        interface = management::GetGenericInterface(command.first.head_command);
      }

      if (command.first.type == kRequestInterface) {
        if (command.first.domain.type != kArgumentNull) {
          Object domain_obj = FetchObject(command.first.domain, true);
          type_id = domain_obj.GetTypeId();
          interface = management::FindInterface(command.first.head_interface, type_id);
          obj_map.insert(NamedObject(kStrObject, domain_obj));
        }
        else {
          ObjectPointer func_obj_ptr = management::FindObject(command.first.head_interface);
          if (func_obj_ptr != nullptr) {
            if (func_obj_ptr->GetTypeId() == kTypeIdFunction) {
              interface = func_obj_ptr->Cast<Interface>();
            }
            else {
              worker.MakeError(command.first.head_interface + " is not a function object.");
            }
          }
          else {
            interface = management::FindInterface(command.first.head_interface);
          }
        }

        if (worker.error) break;

        if (!interface.Good()) {
          worker.MakeError("Function is not found - " + command.first.head_interface);
        }

        GenerateArgs(interface, command.second, obj_map);

        if (worker.error) break;

        if (interface.GetInterfaceType() == kInterfaceIR) {
          //TODO:Processing return value and recovering last worker at CommandReturn
          ir_stack_.push_back(&interface.GetIR());
          worker_stack_.push(MachineWorker());
          obj_stack_.Push();
          continue;
        }
        else {
          msg = interface.Start(obj_map);
        }

        if (msg.GetLevel() == kStateError) break;

        worker.return_stack.push(msg.GetCode() == kCodeObject ?
          msg.GetObj() : Object());

        obj_map.clear();
        worker.idx += 1;
      }
    }

    return msg;
  }
}