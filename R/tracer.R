#' @importFrom instrumentr create_context set_data
#' @export
create_tracer <- function() {
    context <- create_context(
        # application_load_callback = application_load_callback,
        application_unload_callback = .Call(C_application_unload_callback),
        object_duplicate_callback = .Call(C_get_object_duplicate_callback),
        functions = character(0)
    )
    data <- new.env(parent = emptyenv())
    set_data(context, data)
    context
}
