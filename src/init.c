#define R_NO_REMAP

#include <R.h>
#include <R_ext/Rdynload.h>
#include <Rinternals.h>
#include <stdlib.h> // for NULL
#include "r_callbacks.h"

static const R_CallMethodDef callMethods[] = {
    {"object_duplicate_callback", (DL_FUNC) &r_object_duplicate_callback, 0},
    {"application_unload_callback", (DL_FUNC) &r_application_unload_callback, 0},
    {NULL, NULL, 0}
};

void R_init_vtrace(DllInfo* dll) {
    R_registerRoutines(dll, NULL, callMethods, NULL, NULL);

    R_useDynamicSymbols(dll, FALSE);
}
