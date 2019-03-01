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
    size_t logic_idx;
    size_t idx;
    size_t fn_idx;
    size_t skipping_count;
    GenericToken last_command;
    MachineMode mode;
    string error_string;
    Message msg;
    stack<Object> return_stack;
    stack<MachineMode> mode_stack;
    stack<bool> condition_stack;
    stack<size_t> loop_head;
    stack<size_t> loop_tail;
    vector<string> fn_string_vec;

    MachineWorker() :
      error(false),
      deliver(false),
      activated_continue(false),
      activated_break(false),
      origin_idx(0),
      logic_idx(0),
      idx(0),
      fn_idx(0),
      skipping_count(0),
      last_command(kTokenNull),
      mode(kModeNormal),
      error_string(),
      msg(),
      return_stack(),
      mode_stack(),
      condition_stack(),
      loop_head(),
      loop_tail(),
      fn_string_vec() {}

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

  //Kisaragi Machine Class
  class Machine {
  private:
    Object FetchInterfaceObject(string id, string domain);
    Object FetchObject(Argument &arg, bool checking = false);

    void InitFunctionCatching(ArgumentList args);
    void FinishFunctionCatching(bool closure = false);

    void Skipping(bool enable_terminators, 
      std::initializer_list<GenericToken> terminators = {});

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

    void Preprocessor();
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