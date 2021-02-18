#' @importFrom instrumentr create_context
#' @export
create_tracer <- function() {
    context <- create_context(
        closure_call_entry_callback = .Call(C_closure_call_entry_callback),
        closure_call_exit_callback = .Call(C_closure_call_exit_callback),
        object_duplicate_callback = .Call(C_object_duplicate_callback),
        application_unload_callback = .Call(C_application_unload_callback),
        functions = c("base::library")
    )
    context
}
