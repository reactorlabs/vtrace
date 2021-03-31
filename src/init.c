#define R_NO_REMAP

#include <R.h>
#include <R_ext/Rdynload.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL

#include "r_callbacks.h"

/*
 * TODO
 *  - implementation
 *      - EventTable to track alloc, duplicate, dealloc
 *          - Later: track closure entry/exit
 *      - don't skip "in library" tracing, look into why some allocs/deallocs
 *        are not observed
 *      - maybe have some markers for "phases" of the program
 *          - mark function that marks a vector
 *              - need to recursively mark
 *      - don't create a new Function if a closure is allocated multiple times
 *  - next steps:
 *      - "derivation tree" of vectors, i.e. trace builtins/specials to make a
 *        graph of vectors and what they are derived from
 *  - clang-format and cppcheck
 */

/*
 * Table of methods that can be called from R.
 *
 * The first struct element is the symbol available within R, passed to a
 * `.Call` call. Note that these symbols are automatically prefixed by "C_", as
 * specified in `R/vtrace.R`.
 *
 * The second struct element is a pointer to a wrapper function (in
 * `src/r_callbacks.{h,cpp}`), that returns an R pointer to the C++ function
 * (in `src/callbacks.h`).
 *
 * The third struct element is the number of arguments.
 */
static const R_CallMethodDef callMethods[] = {
    {"add_package", (DL_FUNC) &r_add_package, 0},
    {"get_closure_call_entry_callback", (DL_FUNC) &r_get_closure_call_entry_callback, 0},
    {"get_closure_call_exit_callback", (DL_FUNC) &r_get_closure_call_exit_callback, 0},
    // TODO: object_coerce_callback
    {"get_object_duplicate_callback", (DL_FUNC) &r_get_object_duplicate_callback, 0},
    {"get_application_unload_callback", (DL_FUNC) &r_get_application_unload_callback, 0},
    {"get_variable_definition_callback", (DL_FUNC) &r_get_variable_definition_callback, 0},
    {"get_variable_assignment_callback", (DL_FUNC) &r_get_variable_assignment_callback, 0},
    {"get_variable_lookup_callback", (DL_FUNC) &r_get_variable_lookup_callback, 0},
    {"get_context_entry_callback", (DL_FUNC) &r_get_context_entry_callback, 0},
    {"get_context_exit_callback", (DL_FUNC) &r_get_context_exit_callback, 0},
    {"get_context_jump_callback", (DL_FUNC) &r_get_context_jump_callback, 0},
    {"get_gc_allocation_callback", (DL_FUNC) &r_get_gc_allocation_callback, 0},
    {"get_gc_unmark_callback", (DL_FUNC) &r_get_gc_unmark_callback, 0},
    {NULL, NULL, 0} // null terminator
};

/*
 * When the package is loaded, call this init function.
 *
 * R_useDynamicSymbols means only registered functions can be called, and
 * R_forceSymbols means `.Call` must be provided an R symbol, not a string.
 *
 * E.g.: if "name" is registered and the prefix is "C_", then `.Call(C_name)`
 * works but `.Call("name")` does not.
 */
void R_init_vtrace(DllInfo* dll) {
    R_registerRoutines(dll, NULL, callMethods, NULL, NULL);
    R_useDynamicSymbols(dll, FALSE);
    R_forceSymbols(dll, TRUE);
}
