#include <vector>
#include <fstream>
#include <iostream>
#include <Rinternals.h>
#include "callbacks.h"
#include "r_callbacks.h"

#include "model/FunctionTable.h"
#include "model/VectorTable.h"
#include "model/Stack.h"

// TODO: lots of refactoring/rewriting needed here

std::vector<std::string> input_addr;
std::vector<std::string> output_addr;
std::vector<std::string> type;
std::vector<std::string> length;
std::vector<std::string> top_function;
std::vector<std::string> function_id;
char buffer[1024];

bool loaded = false;
int in_library = 0;

FunctionTable function_table;
VectorTable vector_table;
Stack stack;

bool is_library_function(const Function* function) {
    return function->get_name() == "library" &&
           function->get_package_name() == "base";
}

SEXP r_add_package() {
    function_table.update_packages();
    return R_NilValue;
}

void closure_call_entry_callback(ContextSPtr /* context */,
                                 ApplicationSPtr /* application */,
                                 SEXP r_call,
                                 SEXP r_op,
                                 SEXP r_args,
                                 SEXP r_rho) {
    Function* function = function_table.lookup(r_op);
    function -> called();

    StackFrame frame =
        StackFrame::from_call(new Call(function, r_call, r_args, r_rho));

    stack.push(frame);

    if (is_library_function(function)) {
        ++in_library;
    }
}

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

void object_duplicate_callback(ContextSPtr /* context */,
                               ApplicationSPtr /* application */,
                               SEXP r_input,
                               SEXP r_output,
                               SEXP /* r_deep */) {
    if (in_library != 0) return;

    auto t = TYPEOF(r_input);
    if (t == INTSXP || t == REALSXP || t == CPLXSXP || t == LGLSXP ||
        t == RAWSXP || t == STRSXP || t == VECSXP) {
        // TODO: older stuff, record data about vector duplication
        // Most of this handling should be done by the VectorTable
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
        // TODO: VectorTable should be responsible for Vector construction
        Vector* src = vector_table.lookup_by_addr(input);
        Vector* dst = vector_table.lookup_by_addr(output);
        if (!src) {
            src = new Vector(input, TYPEOF(r_input), Rf_length(r_input));
            vector_table.insert(src);
        }
        if (!dst) {
            dst = new Vector(output, TYPEOF(r_input), Rf_length(r_input));
            vector_table.insert(dst);
        }
        dst->set_copy_of(src->get_id());
    }
}

void application_unload_callback(ContextSPtr /* context */,
                                 ApplicationSPtr /* application */) {
    std::ofstream file("duplication.csv");

    file << "input_addr,output_addr,type,length,fun,fun_id\n";
    for (unsigned int i = 0; i < input_addr.size(); ++i) {
        file << input_addr[i] << "," << output_addr[i] << "," << type[i] << ","
             << length[i] << "," << top_function[i] << "," << function_id[i] << "\n";
    }
    file.close();

    std::ofstream file2("duplication_functions.csv");
    function_table.dump_table_to_csv(file2);
    file2.close();


    std::ofstream file3("duplication_vectors.csv");
    vector_table.dump_table_to_csv(file3);
    file3.close();
}

void variable_definition_callback(ContextSPtr /* context */,
                                  ApplicationSPtr /* application */,
                                  SEXP r_symbol,
                                  SEXP r_value,
                                  SEXP r_rho) {
    function_table.update(r_value, CHAR(PRINTNAME(r_symbol)), r_rho);
}

void variable_assignment_callback(ContextSPtr /* context */,
                                  ApplicationSPtr /* application */,
                                  SEXP r_symbol,
                                  SEXP r_value,
                                  SEXP r_rho) {
    function_table.update(r_value, CHAR(PRINTNAME(r_symbol)), r_rho);
}

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

void gc_allocation_callback(ContextSPtr /* context */,
                            ApplicationSPtr /* application */,
                            SEXP r_object) {
    if (TYPEOF(r_object) == CLOSXP) {
        // TODO: check if function was already inserted?
        // Are functions gc'd? Are addresses reused for functions?
        function_table.insert(r_object);
    }
}

void gc_unmark_callback(ContextSPtr /* context */,
                        ApplicationSPtr /* application */,
                        SEXP r_object) {
    if (TYPEOF(r_object) == CLOSXP) {
        function_table.lookup(r_object)->finalize();
        // WARN: causes segfault when run
        //function_table.remove(r_object);
    }
}
