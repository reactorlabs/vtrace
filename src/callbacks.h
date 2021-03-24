#ifndef VTRACE_CALLBACKS_H
#define VTRACE_CALLBACKS_H

#include <Context.hpp>
#include <Application.hpp>

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

void variable_definition_callback(ContextSPtr context,
                                  ApplicationSPtr application,
                                  SEXP r_symbol,
                                  SEXP r_value,
                                  SEXP r_rho);

void variable_assignment_callback(ContextSPtr context,
                                  ApplicationSPtr application,
                                  SEXP r_symbol,
                                  SEXP r_value,
                                  SEXP r_rho);

void variable_lookup_callback(ContextSPtr context,
                              ApplicationSPtr application,
                              SEXP r_symbol,
                              SEXP r_value,
                              SEXP r_rho);

void context_entry_callback(ContextSPtr context,
                            ApplicationSPtr application,
                            void* call_context);

void context_exit_callback(ContextSPtr context,
                           ApplicationSPtr application,
                           void* call_context);

void context_jump_callback(ContextSPtr context,
                           ApplicationSPtr application,
                           void* call_context);

void gc_allocation_callback(ContextSPtr context,
                          ApplicationSPtr application,
                          SEXP r_object);

void gc_unmark_callback(ContextSPtr context,
                        ApplicationSPtr application,
                        SEXP r_object);

#endif // VTRACE_CALLBACKS_H
