#ifndef VTRACE_CALLBACKS_H
#define VTRACE_CALLBACKS_H

#include "Context.hpp"
#include "Application.hpp"

using instrumentr::ApplicationSPtr;
using instrumentr::ContextSPtr;

void closure_call_entry_callback(ContextSPtr context,
                                    ApplicationSPtr application,
                                    SEXP r_call,
                                    SEXP r_op,
                                    SEXP r_args,
                                    SEXP r_rho);

void closure_call_exit_callback(ContextSPtr context,
                                    ApplicationSPtr application,
                                    SEXP r_call,
                                    SEXP r_op,
                                    SEXP r_args,
                                    SEXP r_rho,
                                    SEXP r_result);

void object_duplicate_callback(ContextSPtr context,
                                 ApplicationSPtr application,
                                 SEXP r_input,
                                 SEXP r_output,
                                 SEXP r_deep);

void application_unload_callback(ContextSPtr context,
                                    ApplicationSPtr application);

#endif /* VTRACE_CALLBACKS_H */
