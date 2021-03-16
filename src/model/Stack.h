#ifndef PROMISEDYNTRACER_STACK_H
#define PROMISEDYNTRACER_STACK_H

#include "StackFrame.h"

#include <vector>

class Stack {
  public:
    explicit Stack(): stack_() {
    }

    size_t size() const {
        return stack_.size();
    }

    bool is_empty() const {
        return stack_.empty();
    }

    void push(StackFrame& frame) {
        stack_.push_back(frame);
    }

    StackFrame pop() {
        StackFrame frame{peek(1)};
        stack_.pop_back();
        return frame;
    }

    const StackFrame& peek(std::size_t n = 1) const {
        return stack_[stack_.size() - n];
    }

    StackFrame& peek(std::size_t n = 1) {
        return stack_[stack_.size() - n];
    }

    Call* topmost_call() {
        for (auto it = stack_.rbegin(); it != stack_.rend(); ++it) {
            if (it->is_call()) {
                return it->as_call();
            }
        }
        return nullptr;
    }


  private:
    std::vector<StackFrame> stack_;
};

#endif /* PROMISEDYNTRACER_EXECUTION_CONTEXT_STACK_H */
