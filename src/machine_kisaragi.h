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
    bool deliver;
    bool activated_continue;
    bool activated_break;
    size_t origin_idx;
    size_t idx;
    GenericToken last_command;
    MachineMode mode;
    string error_string;
    Message msg;
    stack<Object> return_stack;
    stack<MachineMode> mode_stack;
    stack<bool> condition_stack;
    stack<size_t> loop_head;
    stack<size_t> loop_tail;

    MachineWorker() :
      origin_idx(0),
      idx(0) {}

    void MakeError(string str) {
      error = true;
      error_string = str;
    }

    void MakeMsg(Message msg) {
      deliver = true;
      this->msg = msg;
    }

    Message GetMsg() {
      deliver = false;
      return msg;
    }

    void SwitchToMode(MachineMode mode) {
      mode_stack.push(this->mode);
      this->mode = mode;
    }

    void GoLastMode() {
      if (!mode_stack.empty()) {
        this->mode = mode_stack.top();
        mode_stack.pop();
      }
      else {
        MakeError("Mode switching error.Kisaragi Kernel Panic.");
      }
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
    void CommandIfOrWhile(GenericToken token, ArgumentList args);
    void CommandElse(ArgumentList args);
    void CommandConditionEnd();
    void CommandLoopEnd();

    void CommandReturn(ArgumentList args);
    void MachineCommands(GenericToken token, ArgumentList args);

    void GenerateArgs(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Generate_Normal(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Generate_AutoSize(Interface &interface, ArgumentList args, ObjectMap &obj_map);
    void Generate_AutoFill(Interface &interface, ArgumentList args, ObjectMap &obj_map);
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

    void SetPreviousStack(ObjectStack &prev) {
      obj_stack_.SetPreviousStack(prev);
    }

    Message Run();
  };
}