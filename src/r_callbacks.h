#ifndef VTRACE_R_CALLBACKS_H
#define VTRACE_R_CALLBACKS_H

#include "Rincludes.h"

#ifdef __cplusplus
extern "C" {
#endif
SEXP r_closure_call_entry_callback();
SEXP r_closure_call_exit_callback();
SEXP r_object_duplicate_callback();
SEXP r_application_unload_callback();
#ifdef __cplusplus
}
#endif

#endif /* VTRACE_R_CALLBACKS_H  */
