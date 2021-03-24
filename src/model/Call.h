#ifndef VTRACE_CALL_H
#define VTRACE_CALL_H

#include "Function.h"

/*
 * This class models R calls, i.e. a function that is being called. A Call
 * consists of a Function (the tracer's model of "op", the function being
 * called), the call expression (r_call), its arguments (r_args), and the
 * environment the call is being evaluated in (r_rho).
 *
 * In the tracer, Calls are wrapped as StackFrames and placed on the Stack.
 */
class Call {
    Function* function_;
    SEXP r_call_;
    SEXP r_args_;
    SEXP r_rho_;
    bool interrupted_ = false;

  public:
    Call(Function* function, SEXP r_call, SEXP r_args, SEXP r_rho)
        : function_(function)
        , r_call_(r_call)
        , r_args_(r_args)
        , r_rho_(r_rho) { }

    Function* get_function() {
        return function_;
    }

    const Function* get_function() const {
        return function_;
    }

    SEXP get_expression() {
        return r_call_;
    }

    SEXP get_arguments() {
        return r_args_;
    }

    SEXP get_environment() {
        return r_rho_;
    }

    void set_interrupted() {
        interrupted_ = true;
    }

    bool is_interrupted() const {
        return interrupted_;
    }
};

#endif // VTRACE_CALL_H
