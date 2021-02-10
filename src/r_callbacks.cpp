#include "r_callbacks.h"
#include "callbacks.h"

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
