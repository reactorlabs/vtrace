#define R_NO_REMAP

#include <R.h>
#include <R_ext/Rdynload.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include "r_callbacks.h"

static const R_CallMethodDef callMethods[] = {
    {"add_package", (DL_FUNC) &r_add_package, 0},
    {"closure_call_entry_callback", (DL_FUNC) &r_closure_call_entry_callback, 0},
    {"closure_call_exit_callback", (DL_FUNC) &r_closure_call_exit_callback, 0},
    {"object_duplicate_callback", (DL_FUNC) &r_object_duplicate_callback, 0},
    {"application_unload_callback", (DL_FUNC) &r_application_unload_callback, 0},
    {"get_variable_definition_callback", (DL_FUNC) &r_get_variable_definition_callback, 0},
    {"get_variable_assignment_callback", (DL_FUNC) &r_get_variable_assignment_callback, 0},
    {"get_variable_lookup_callback", (DL_FUNC) &r_get_variable_lookup_callback, 0},
    {"get_context_entry_callback", (DL_FUNC) &r_get_context_entry_callback, 0},
    {"get_context_exit_callback", (DL_FUNC) &r_get_context_exit_callback, 0},
    {"get_context_jump_callback", (DL_FUNC) &r_get_context_jump_callback, 0},
    {"get_gc_allocation_callback", (DL_FUNC) &r_get_gc_allocation_callback, 0},
    {"get_gc_unmark_callback", (DL_FUNC) &r_get_gc_unmark_callback, 0},
    {NULL, NULL, 0}
};

void R_init_vtrace(DllInfo* dll) {
    R_registerRoutines(dll, NULL, callMethods, NULL, NULL);

    R_useDynamicSymbols(dll, FALSE);
}
