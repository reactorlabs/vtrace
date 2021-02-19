#include <vector>
#include <fstream>
#include <iostream>
#include "Rinternals.h"
#include "callbacks.h"

#include "FunctionTable.h"
#include "Stack.h"

std::vector<std::string> input_addr;
std::vector<std::string> output_addr;
std::vector<std::string> type;
std::vector<std::string> length;
std::vector<std::string> top_function;
char buffer[1024];

bool loaded = false;
int in_library = 0;

FunctionTable function_table;
Stack stack;

bool is_library_function(const Function* function) {
    return function->get_name() == "library" &&
           function->get_package_name() == "base";
}

void closure_call_entry_callback(ContextSPtr context,
                                 ApplicationSPtr application,
                                 SEXP r_call,
                                 SEXP r_op,
                                 SEXP r_args,
                                 SEXP r_rho) {
    Function* function = function_table.lookup(r_op);

    StackFrame frame =
        StackFrame::from_call(new Call(function, r_call, r_args, r_rho));

    stack.push(frame);

    if (!in_library) {
        std::cout << "Entering: "
                  << "[" << in_library << "]" << function->get_qualified_name()
                  << "\n";
    }

    if (is_library_function(function)) {
        ++in_library;
    }
}

void closure_call_exit_callback(ContextSPtr context,
                                ApplicationSPtr application,
                                SEXP r_call,
                                SEXP r_op,
                                SEXP r_args,
                                SEXP r_rho,
                                SEXP r_result) {
    Function* function = function_table.lookup(r_op);

    if (is_library_function(function)) {
        --in_library;
    }

    StackFrame frame = stack.pop();

    if (!frame.is_call()) {
        Rf_error("mismatched stack frame, expected call got context");
    } else {
        Call* call = frame.as_call();
        if (call->get_expression() != r_call ||
            call->get_arguments() != r_args ||
            call->get_environment() != r_rho) {
            Rf_error("mismatched call on stack");
        }
        delete call;
    }

    if (!in_library) {
        std::cout << "Exiting: "
                  << "[" << in_library << "]" << function->get_qualified_name()
                  << "\n";
    }
}

void object_duplicate_callback(ContextSPtr context,
                               ApplicationSPtr application,
                               SEXP r_input,
                               SEXP r_output,
                               SEXP r_deep) {
    if (in_library != 0) return;

    // std::cout << "In: " << in_library << "\n";

    auto t = TYPEOF(r_input);
    if (t == INTSXP || t == REALSXP || t == CPLXSXP || t == LGLSXP ||
        t == RAWSXP || t == STRSXP || t == VECSXP) {
        sprintf(buffer, "%p", r_input);
        input_addr.push_back(std::string(buffer));

        sprintf(buffer, "%p", r_output);
        output_addr.push_back(std::string(buffer));

        type.push_back(std::string(type2char(TYPEOF(r_input))));

        length.push_back(std::to_string(Rf_length(r_input)));

        if (auto *call = stack.topmost_call()) {
            auto name = std::string(call->get_function()->get_qualified_name());
            if (name == "<unknown>") {
                name = call->get_function()->get_definition();
            }
            top_function.push_back(name);
        } else {
            top_function.push_back("NA");
        }
    }
}

void application_unload_callback(ContextSPtr context,
                                 ApplicationSPtr application) {
    std::ofstream file("duplication.csv");

    file << "input_addr,output_addr,type,length,fun\n";
    for (int i = 0; i < input_addr.size(); ++i) {
        file << input_addr[i] << "," << output_addr[i] << "," << type[i] << ","
             << length[i] << "," << top_function[i] << "\n";
    }
}

void variable_definition_callback(ContextSPtr context,
                                  ApplicationSPtr application,
                                  SEXP r_symbol,
                                  SEXP r_value,
                                  SEXP r_rho) {
    function_table.update(r_value, CHAR(PRINTNAME(r_symbol)), r_rho);
}

void variable_assignment_callback(ContextSPtr context,
                                  ApplicationSPtr application,
                                  SEXP r_symbol,
                                  SEXP r_value,
                                  SEXP r_rho) {
    function_table.update(r_value, CHAR(PRINTNAME(r_symbol)), r_rho);
}

void variable_lookup_callback(ContextSPtr context,
                              ApplicationSPtr application,
                              SEXP r_symbol,
                              SEXP r_value,
                              SEXP r_rho) {
    function_table.update(r_value, CHAR(PRINTNAME(r_symbol)), r_rho);
}

void context_entry_callback(ContextSPtr context,
                            ApplicationSPtr application,
                            void* call_context) {
    StackFrame frame = StackFrame::from_context(call_context);
    stack.push(frame);
}

void context_exit_callback(ContextSPtr context,
                           ApplicationSPtr application,
                           void* call_context) {
    StackFrame frame = stack.pop();

    if (!frame.is_context()) {
        Rf_error("mismatched stack frame, expected context got call");
    } else if (frame.as_context() != call_context) {
        Rf_error("mismatched context on stack, expected %p got %p",
                 call_context,
                 frame.as_context());
    }
}

void context_jump_callback(ContextSPtr context,
                           ApplicationSPtr application,
                           void* call_context) {
    while (stack.size()) {
        StackFrame& frame = stack.peek();

        if (frame.is_context()) {
            if (frame.as_context() == call_context) {
                return;
            }

            else {
                void* call_context = frame.as_context();
                context_exit_callback(context, application, call_context);
            }
        }

        else if (frame.is_call()) {
            Call* call = frame.as_call();

            SEXP r_call = call->get_expression();
            SEXP r_op = call->get_function()->get_op();
            SEXP r_args = call->get_arguments();
            SEXP r_rho = call->get_environment();

            closure_call_exit_callback(
                context, application, r_call, r_op, r_args, r_rho, NULL);
        }
    }

    Rf_error("cannot find matching context while unwinding\n");
}

void gc_allocation_callback(ContextSPtr context,
                            ApplicationSPtr application,
                            SEXP r_object) {
    if (TYPEOF(r_object) == CLOSXP) {
        function_table.insert(r_object);
    }
}

void gc_unmark_callback(ContextSPtr context,
                        ApplicationSPtr application,
                        SEXP r_object) {
    if (TYPEOF(r_object) == CLOSXP) {
        // WARN: causes segfault when run
        //function_table.remove(r_object);
    }
}
