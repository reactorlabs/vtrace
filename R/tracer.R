#' @importFrom instrumentr create_context
#' @export
create_tracer <- function() {
    context <- create_context(
        object_duplicate_callback = .Call(C_object_duplicate_callback),
        application_unload_callback = .Call(C_application_unload_callback),
        functions = character(0)
    )
    context
}
