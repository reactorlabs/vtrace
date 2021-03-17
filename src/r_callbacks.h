#ifndef VTRACE_R_CALLBACKS_H
#define VTRACE_R_CALLBACKS_H

#include <Rincludes.h>

#ifdef __cplusplus
extern "C" {
#endif
// r_add_package is defined in `callbacks.cpp`; it doesn't need a wrapper.
SEXP r_add_package();
SEXP r_get_closure_call_entry_callback();
SEXP r_get_closure_call_exit_callback();
SEXP r_get_object_duplicate_callback();
SEXP r_get_application_unload_callback();
SEXP r_get_variable_definition_callback();
SEXP r_get_variable_assignment_callback();
SEXP r_get_variable_lookup_callback();
SEXP r_get_context_entry_callback();
SEXP r_get_context_exit_callback();
SEXP r_get_context_jump_callback();
SEXP r_get_gc_allocation_callback();
SEXP r_get_gc_unmark_callback();
#ifdef __cplusplus
}
#endif

#endif // VTRACE_R_CALLBACKS_H
