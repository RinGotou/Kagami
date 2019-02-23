#pragma once
/*
   Kisaragi(February) - Next Kagami IR Interpreter Framework
*/
#include "trace.h"
#include "management.h"

namespace kagami {
  using CombinedCode = pair<size_t, string>;

  class MachineWorker {
  public:
    bool error;
    size_t origin_idx;
    size_t idx;
    GenericToken last_command;
    MachineMode mode;
    string error_string;
    stack<Object> return_stack;
    
    MachineWorker() :
      origin_idx(0),
      idx(0) {}

    void MakeError(string str) {
      error = true;
      error_string = str;
    }

    bool NeedSkipping() {
      switch (mode) {
      case kModeNextCondition:
      case kModeCycleJump:
      case kModeDef:
      case kModeCaseJump:
      case kModeClosureCatching:
        return true;
        break;
      default:
        break;
      }
      return false;
    }
  };

  class IRLoader {
    
  };

  class Machine {
  private:
    Object FetchInterfaceObject(string id, string domain);
    Object FetchObject(Argument &arg, bool checking = false);
    void SetSegmentInfo(ArgumentList args);

    void MachineCommands(GenericToken token, ArgumentList args) {
      switch (token) {
      case kTokenSegment:
        SetSegmentInfo(args);
        break;

      default:
        break;
      }
    }
  private:
    deque<KIRPointer> ir_stack_;
    stack<MachineWorker> worker_stack_;
    ObjectStack obj_stack_;
    
  public:
    Machine() {}

    Machine(const Machine &rhs) :
      ir_stack_(rhs.ir_stack_),
      worker_stack_(rhs.worker_stack_),
      obj_stack_(rhs.obj_stack_) {}

    Machine(const Machine &&rhs) :
      Machine(rhs) {}

    Machine(KIR &ir) {
      ir_stack_.push_back(&ir);
    }

    Message Run(string name = "");
  };
}