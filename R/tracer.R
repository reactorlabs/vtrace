#' @importFrom instrumentr create_context
#' @export
create_tracer <- function() {
    context <- create_context(
        application_load_callback = application_load_callback,
        closure_call_entry_callback = .Call(C_closure_call_entry_callback),
        closure_call_exit_callback = .Call(C_closure_call_exit_callback),
        object_duplicate_callback = .Call(C_object_duplicate_callback),
        application_detach_callback = application_detach_callback,
        application_unload_callback = .Call(C_application_unload_callback),

        context_entry_callback = .Call(C_get_context_entry_callback),
        context_exit_callback = .Call(C_get_context_exit_callback),
        context_jump_callback = .Call(C_get_context_jump_callback),

        variable_definition_callback = .Call(C_get_variable_definition_callback),
        variable_assignment_callback = .Call(C_get_variable_assignment_callback),
        variable_lookup_callback = .Call(C_get_variable_lookup_callback),

        gc_allocation_callback = .Call(C_get_gc_allocation_callback),
        gc_unmark_callback = .Call(C_get_gc_unmark_callback)
    )
    context
}

#' @importFrom instrumentr trace_code
#' @export
trace_code <- function(code,
                       envir = parent.frame(),
                       quote=TRUE) {

    force_lazy_loaded_functions(search())

    context <- create_tracer()

    if (quote) {
        code <- substitute(code)
    }

    instrumentr::trace_code(context, code, envir, quote = FALSE)
}


force_lazy_loaded_functions <- function(packages) {
    for(package in packages) {
        if(startsWith(package, "package:")) {
            ns <- getNamespace(substr(package, 9, nchar(package)))
            names <- ls(ns, all.names = TRUE)
            Map(function(name) get(name, envir = ns), names)
        }
    }

    NULL
}

application_load_callback <- function(context, application) {
    installed_packages <- installed.packages()[,1]
    for (pkg in installed_packages) {
        setHook(packageEvent(pkg, "onLoad"), add_package, "prepend")
    }
}

application_detach_callback <- function(context, application) {
    installed_packages <- installed.packages()[,1]
    for (pkg in installed_packages) {
        setHook(packageEvent(pkg, "onLoad"), NULL, "replace")
    }
}

add_package <- function(pkg_name, lib_name) {
    cat("ADD_PACKAGE: ", pkg_name, "\n")
    force_lazy_loaded_functions(paste0("package:", pkg_name))
    .Call("C_add_package", PACKAGE="vtrace")
}
