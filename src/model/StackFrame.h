#ifndef VTRACE_STACK_FRAME_H
#define VTRACE_STACK_FRAME_H

#include "Call.h"

/*
 * This class models stack frames on the R stack. A stack frame may be a call
 * (representing an R function) or a context (used by R to implement non-local
 * returns). This is implemented as a tagged union.
 *
 * Note that native calls are not modelled.
 *
 * The constructor is private, because a StackFrame must be created using one
 * of the static methods, to ensure we properly construct a call or context
 * frame.
 */
class StackFrame {
    enum class Type { Call, Context };

    Type type_;
    union {
        Call* call_;
        void* context_;
    };

  public:
    static StackFrame from_call(Call* call) {
        return StackFrame(call, Type::Call);
    }

    static StackFrame from_context(void* context) {
        return StackFrame(context, Type::Context);
    }

    bool is_call() const {
        return type_ == Type::Call;
    }

    Call* as_call() {
        return call_;
    }

    const Call* as_call() const {
        return call_;
    }

    bool is_context() const {
        return type_ == Type::Context;
    }

    void* as_context() {
        return context_;
    }

    const void* as_context() const {
        return context_;
    }

  private:
    StackFrame(void* ptr, Type type) : type_(type) {
        switch (type) {
        case Type::Call:
            call_ = static_cast<Call*>(ptr);
            break;
        case Type::Context:
            context_ = ptr;
            break;
        }
    }
};

#endif // VTRACE_STACK_FRAME_H
