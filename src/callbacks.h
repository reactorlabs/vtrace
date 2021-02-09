#ifndef VTRACE_CALLBACKS_H
#define VTRACE_CALLBACKS_H

#include "Context.hpp"
#include "Application.hpp"

using instrumentr::ApplicationSPtr;
using instrumentr::ContextSPtr;

void object_duplicate_callback(ContextSPtr context,
                                 ApplicationSPtr application,
                                 SEXP r_input,
                                 SEXP r_output,
                                 SEXP r_deep);

void application_unload_callback(ContextSPtr context,
                                    ApplicationSPtr application);

#endif /* VTRACE_CALLBACKS_H */
