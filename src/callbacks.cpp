#include <vector>
#include <fstream>
#include <iostream>
#include <Rinternals.h>

#include "callbacks.h"
#include "r_callbacks.h"

#include "model/FunctionTable.h"
#include "model/VectorTable.h"
#include "model/Stack.h"

/*
 * This file contains the implementation of the native callbacks for the
 * tracer.
 */

// TODO: lots of refactoring/rewriting needed here

// TODO: this will be handled by an EventTable
std::vector<std::string> input_addr;
std::vector<std::string> output_addr;
std::vector<std::string> type;
std::vector<std::string> length;
std::vector<std::string> top_function;
std::vector<std::string> function_id;
char buffer[1024];

int in_library = 0;

FunctionTable function_table;
VectorTable vector_table;
Stack stack;

// If the function name is "library" and it is in the base package, assume it
// is the function that loads and attaches packages.
//
// This function will likely be removed in the future, when we also trace
// library loading.
bool is_library_function(const Function* function) {
    return function->get_name() == "library" &&
           function->get_package_name() == "base";
}

// When an R package is loaded, the tracer needs to know about it, so it can
// update the FunctionTable.
SEXP r_add_package() {
    function_table.update_packages();
    return R_NilValue;
}

// When a closure is called, we update the FunctionTable and push a new
// call frame onto the model stack.
void closure_call_entry_callback(ContextSPtr /* context */,
                                 ApplicationSPtr /* application */,
                                 SEXP r_call,
                                 SEXP r_op,
                                 SEXP r_args,
                                 SEXP r_rho) {
    Function* function = function_table.lookup(r_op);
    function->called();

    StackFrame frame =
        StackFrame::from_call(new Call(function, r_call, r_args, r_rho));

    stack.push(frame);

    if (is_library_function(function)) {
        ++in_library;
    }
}

// When a closure is exited, we pop the model stack and check that there is no
// stack frame mismatch.
void closure_call_exit_callback(ContextSPtr /* context */,
                                ApplicationSPtr /* application */,
                                SEXP r_call,
                                SEXP r_op,
                                SEXP r_args,
                                SEXP r_rho,
                                SEXP /* r_result */) {
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
}

// Record object duplication, but only if it is a vector.
//
// TODO: figure out what vector_copy and matrix_copy do.
void object_duplicate_callback(ContextSPtr /* context */,
                               ApplicationSPtr /* application */,
                               SEXP r_input,
                               SEXP r_output,
                               SEXP /* r_deep */) {
    if (in_library != 0) return;

    if (Vector::is_vector(r_input)) {
        // TODO: older stuff, record data about vector duplication
        // Most of this handling should be done by the VectorTable
        // Or moved into an EventTable for "duplication" events
        sprintf(buffer, "%p", (void*)r_input);
        std::string input = std::string(buffer);
        input_addr.push_back(input);

        sprintf(buffer, "%p", (void*)r_output);
        std::string output = std::string(buffer);
        output_addr.push_back(output);

        type.push_back(std::string(type2char(TYPEOF(r_input))));

        length.push_back(std::to_string(Rf_length(r_input)));

        if (auto *call = stack.topmost_call()) {
            auto name = std::string(call->get_function()->get_qualified_name());
            top_function.push_back(name);
            function_id.push_back(std::to_string(call->get_function()->get_id()));
        } else {
            top_function.push_back("NA");
            function_id.push_back("NA");
        }

        // newer stuff, create vector objects and add to vector table
        vector_table.duplicate(r_input, r_output);
    }
}

// When the vtrace package is being unloaded, dump the data out to CSV files.
void application_unload_callback(ContextSPtr /* context */,
                                 ApplicationSPtr /* application */) {
    std::ofstream file("duplication.csv");

    file << "input_addr,output_addr,type,length,fun,fun_id\n";
    for (unsigned int i = 0; i < input_addr.size(); ++i) {
        file << input_addr[i] << "," << output_addr[i] << "," << type[i] << ","
             << length[i] << "," << top_function[i] << "," << function_id[i] << "\n";
    }
    file.close();

    std::ofstream file2("functions.csv");
    function_table.dump_table_to_csv(file2);
    file2.close();

    std::ofstream file3("vectors.csv");
    vector_table.dump_table_to_csv(file3);
    file3.close();
}

// This might involve binding a function to a name, so update the function
// table.
void variable_definition_callback(ContextSPtr /* context */,
                                  ApplicationSPtr /* application */,
                                  SEXP r_symbol,
                                  SEXP r_value,
                                  SEXP r_rho) {
    function_table.update(r_value, CHAR(PRINTNAME(r_symbol)), r_rho);
}

// This might involve binding a function to a name, so update the function
// table.
void variable_assignment_callback(ContextSPtr /* context */,
                                  ApplicationSPtr /* application */,
                                  SEXP r_symbol,
                                  SEXP r_value,
                                  SEXP r_rho) {
    function_table.update(r_value, CHAR(PRINTNAME(r_symbol)), r_rho);
}

// This might involve binding a function to a name, so update the function
// table.
void variable_lookup_callback(ContextSPtr /* context */,
                              ApplicationSPtr /* application */,
                              SEXP r_symbol,
                              SEXP r_value,
                              SEXP r_rho) {
    function_table.update(r_value, CHAR(PRINTNAME(r_symbol)), r_rho);
}

void context_entry_callback(ContextSPtr /* context */,
                            ApplicationSPtr /* application */,
                            void* call_context) {
    StackFrame frame = StackFrame::from_context(call_context);
    stack.push(frame);
}

void context_exit_callback(ContextSPtr /* context */,
                           ApplicationSPtr /* application */,
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

// A non-local return has occurred, so unwind the stack until we reach the
// target context.
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

// Object allocation callback. For now, we only care about functions and
// vectors.
void gc_allocation_callback(ContextSPtr /* context */,
                            ApplicationSPtr /* application */,
                            SEXP r_object) {
    if (TYPEOF(r_object) == CLOSXP) {
        // TODO: check if function was already inserted?
        // Are functions gc'd? Are addresses reused for functions?
        function_table.insert(r_object);
    } else if (Vector::is_vector(r_object)) {
        if (in_library != 0) return;
        vector_table.insert(r_object);
    }
}

// Object deallocation callback.
// NOTE: this callback must not allocate R memory, otherwise a recusrive GC
// invocation may occur!
void gc_unmark_callback(ContextSPtr /* context */,
                        ApplicationSPtr /* application */,
                        SEXP r_object) {
    if (TYPEOF(r_object) == CLOSXP) {
        if (auto function = function_table.lookup_no_create(r_object)) {
            function->finalize();
            // WARN: causes segfault when run
            //function_table.remove(r_object);
        }
    } else if (Vector::is_vector(r_object)) {
        vector_table.finalize(r_object);
    }
}
