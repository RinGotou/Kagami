#include "machine_kisaragi.h"

namespace kagami {
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
    MachineWorker &worker = worker_stack_.top();
    stack<Object> &return_stack = worker_stack_.top().return_stack;

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

  Message Machine::Run(string name) {
    if (ir_stack_.empty()) return Message();
    StateLevel level = kStateNormal;
    StateCode code = kCodeSuccess;
    string detail;
    Message msg;
    KIR &ir = *ir_stack_.back();

    worker_stack_.push(MachineWorker());
    obj_stack_.Push();

    MachineWorker &worker = worker_stack_.top();
    size_t size = ir.size();

    while (worker.idx < size) {
      if (worker.NeedSkipping()) {

      }

      
    }
  }
}