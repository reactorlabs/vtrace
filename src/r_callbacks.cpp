#include "callbacks.h"
#include "r_callbacks.h"

// r_add_package is defined in `callbacks.cpp`; it doesn't need a wrapper.

SEXP r_closure_call_entry_callback() {
    return R_MakeExternalPtr(
        (void*) (closure_call_entry_callback), R_NilValue, R_NilValue);
}

SEXP r_closure_call_exit_callback() {
    return R_MakeExternalPtr(
        (void*) (closure_call_exit_callback), R_NilValue, R_NilValue);
}

SEXP r_object_duplicate_callback() {
    return R_MakeExternalPtr(
        (void*) (object_duplicate_callback), R_NilValue, R_NilValue);
}

SEXP r_application_unload_callback() {
    return R_MakeExternalPtr(
        (void*) (application_unload_callback), R_NilValue, R_NilValue);
}

SEXP r_get_variable_definition_callback() {
    return R_MakeExternalPtr(
        (void*) (variable_definition_callback), R_NilValue, R_NilValue);
}

SEXP r_get_variable_assignment_callback() {
    return R_MakeExternalPtr(
        (void*) (variable_assignment_callback), R_NilValue, R_NilValue);
}

SEXP r_get_variable_lookup_callback() {
    return R_MakeExternalPtr(
        (void*) (variable_lookup_callback), R_NilValue, R_NilValue);
}

SEXP r_get_context_entry_callback() {
    return R_MakeExternalPtr(
        (void*) (context_entry_callback), R_NilValue, R_NilValue);
}

SEXP r_get_context_exit_callback() {
    return R_MakeExternalPtr(
        (void*) (context_exit_callback), R_NilValue, R_NilValue);
}

SEXP r_get_context_jump_callback() {
    return R_MakeExternalPtr(
        (void*) (context_jump_callback), R_NilValue, R_NilValue);
}

SEXP r_get_gc_allocation_callback() {
    return R_MakeExternalPtr(
        (void*) (gc_allocation_callback), R_NilValue, R_NilValue);
}

SEXP r_get_gc_unmark_callback() {
    return R_MakeExternalPtr(
        (void*) (gc_unmark_callback), R_NilValue, R_NilValue);
}
