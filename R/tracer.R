# Creates and returns the tracer object by setting up the callbacks. Most
# callbacks are implemented in C.
#
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

# Helper function that traces the provided code. This helper searches all
# attached packages (returned by the call to `search()`), forcing functions
# that would be lazy loaded, then initializes the tracer object, and then runs
# the instrumentr tracer.
#
#' @importFrom instrumentr trace_code
#' @export
trace_code <- function(code,
                       envir = parent.frame(),
                       quote = TRUE) {
    force_lazy_loaded_functions(search())
    context <- create_tracer()

    if (quote) {
        code <- substitute(code)
    }

    instrumentr::trace_code(context, code, envir, quote = FALSE)
}

# Traverse the given vector of packages and objects, representing the R search
# path.
force_lazy_loaded_functions <- function(packages) {
    for (package in packages) {
        # Packages are prefixed with "package:".
        if (startsWith(package, "package:")) {
            # Strip the prefix, then get the environment representing the
            # package's namespace.
            ns <- getNamespace(substr(package, 9, nchar(package)))

            # Return all names in the environment, including ones starting
            # with '.'.
            names <- ls(ns, all.names = TRUE)

            # Accessing the object with `get` will force lazy loading; don't
            # care if it's a function or not.
            Map(function(name) get(name, envir = ns), names)
        }
    }

    NULL
}

# Package hooks must be implemented in R. Here, we want the hook to call
# `add_package` every time a package is loaded.
#
# installed.packages returns the details of every package installed on the
# system; the first column gives the package names.
#
# Note that loading (making an installed package available) and attaching
# (adding the package to the search path) are two separate steps, both
# performed by `library()`. We add a hook to the onLoad event, because those
# packages are accessible and we want the tracer to be aware of them.
application_load_callback <- function(context, application) {
    installed_packages <- installed.packages()[, 1]
    for (pkg in installed_packages) {
        # packageEvent creates a hook name, derived from the package name
        setHook(packageEvent(pkg, "onLoad"), add_package, "prepend")
    }
}

# Cleanup: remove all package hooks, by replacing existing hooks with NULL.
# We call this hook when detaching, because we have a separate hook
# (implemented in C) that is called when unloading.
application_detach_callback <- function(context, application) {
    installed_packages <- installed.packages()[, 1]
    for (pkg in installed_packages) {
        # Note: use the hook name created by packageEvent; when we added the
        # hook we used "onLoad" as part of its name, so we have to use "onLoad"
        # here.
        setHook(packageEvent(pkg, "onLoad"), NULL, "replace")
    }
}

# This hook is called when a package is loaded. In this function, we force
# the lazy loaded functions, then call the C function so the tracer refreshes
# its function table.
add_package <- function(pkg_name, lib_name) {
    # force_lazy_loaded_functions expects packages to be prefixed by "package:"
    force_lazy_loaded_functions(paste0("package:", pkg_name))
    .Call(C_add_package, PACKAGE="vtrace")
}
