#ifndef VTRACE_FUNCTION_TABLE_H
#define VTRACE_FUNCTION_TABLE_H

#include <R.h>
#include <Rinternals.h>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "Function.h"

/*
 * The FunctionTable keeps track of all functions we have seen. It is
 * responsible for allocating and deallocating Function objects.
 *
 * The FunctionTable will also try to determine the names of each function.
 * This happens upon initialization, whenever a function is updated, and
 * whenever the package list is updated.
 */
class FunctionTable {
    // Map of SEXP to tracer Function pointers
    std::unordered_map<SEXP, Function*> table_;
    // Set of packages that have already been seen and processed.
    std::unordered_set<SEXP> seen_packages_;

  public:
    // Initialize FunctionTable with all accessible functions
    FunctionTable() {
        update_packages();
    }

    // Delete all the Function objects in the table
    ~FunctionTable() {
        for (auto& it : table_) {
            delete it.second;
        }
    }

    // Insert an SEXP into the table, creating a Function object
    void insert(SEXP r_closure) {
        Function* function = new Function(r_closure);
        auto result = table_.emplace(r_closure, function);

        // If function is already in the table, replace the existing one
        if (!result.second) {
            delete result.first->second;
            result.first->second = function;
        }
    }

    // Remove the SEXP and function object from the table
    void remove(SEXP r_closure) {
        auto result = table_.find(r_closure);

        if (result != table_.end()) {
            Function* function = result->second;
            table_.erase(result);
            delete function;
        }
    }

    // Find or create the Function corresponding to the SEXP
    Function* lookup(SEXP r_closure) {
        return get_or_create_(r_closure);
    }

    // Update the function with a new name, if necessary and possible
    void update(SEXP r_value, const char* name, SEXP r_rho) {
        SEXP r_closure = unwrap_function_(r_value);

        if (TYPEOF(r_closure) != CLOSXP) {
            return;
        }

        Function* function = lookup(r_closure);

        if (function->has_name()) {
            return;
        }

        update_name_(function, name, r_rho);
    }

    // Update the FunctionTable by iterating over all functions in packages that
    // have not yet been seen.
    void update_packages() {
        SEXP r_package_names = R_lsInternal(R_NamespaceRegistry, TRUE);

        // Iterate over the list of packages
        for (int i = 0; i < Rf_length(r_package_names); ++i) {
            const char* name = CHAR(STRING_ELT(r_package_names, i));
            SEXP r_package =
                Rf_findVarInFrame(R_NamespaceRegistry, Rf_install(name));

            if (TYPEOF(r_package) != ENVSXP) {
                continue;
            }

            // Mark package as seen; if it was already seen, continue
            auto result = seen_packages_.emplace(r_package);
            if (!result.second) {
                continue;
            }

            // Iterate over the functions in the environment and update their names
            SEXP r_names = R_lsInternal(r_package, TRUE);
            for (int i = 0; i < Rf_length(r_names); ++i) {
                const char* name = CHAR(STRING_ELT(r_names, i));
                SEXP r_fun = Rf_findVarInFrame(r_package, Rf_install(name));
                update(r_fun, name, r_package);
            }
        }
    }

    // Dump the FunctionTable as a CSV file. Skip functions that were never
    // called.
    void dump_table_to_csv(std::ofstream& file) {
        file << "id,name,hash,definition\n";
        for (const auto& it: table_) {
            auto fun = it.second;
            if (fun->is_called()) {
                file << fun->get_id() << "," << fun->get_name() << ","
                     << fun->get_hash() << "," << fun->get_definition() << "\n";
            }
        }
    }

  private:
    // If the SEXP is a promise, we have to unwrap it
    SEXP unwrap_function_(SEXP r_value) {
        SEXP r_closure = R_NilValue;

        switch (TYPEOF(r_value)) {
        case CLOSXP:
            r_closure = r_value;
            break;
        case PROMSXP:
            r_closure = dyntrace_get_promise_value(r_value);
            if (r_closure == R_UnboundValue || TYPEOF(r_closure) != CLOSXP) {
                r_closure = dyntrace_get_promise_expression(r_value);
                if (r_closure == R_UnboundValue || TYPEOF(r_closure) != CLOSXP) {
                    r_closure = R_NilValue;
                }
            }
            break;
        default:
            break;
        }

        return r_closure;
    }

    SEXP infer_namespace_(SEXP r_package) {
        if (r_package == R_BaseEnv) {
            return R_BaseNamespace;
        } else if (r_package == R_GlobalEnv) {
            return R_GlobalEnv;
        } else if (R_IsNamespaceEnv(r_package)) {
            return r_package;
        } else if (R_IsPackageEnv(r_package)) {
            const char* package_name =
                CHAR(STRING_ELT(R_PackageEnvName(r_package), 0));
            return Rf_findVarInFrame(
                R_NamespaceRegistry,
                Rf_install(package_name + strlen("package:")));
        } else {
            return r_package;
        }
    }

    Function* get_or_create_(SEXP r_closure) {
        auto result = table_.find(r_closure);

        if (result != table_.end()) {
            return result->second;
        } else {
            Function* function = new Function(r_closure);
            table_.emplace(r_closure, function);
            return function;
        }
    }

    // Essentially, if we are updating `r_rho` so that `function` is bound to
    // `name`, and `r_rho` is the lexical environment of the function, then
    // `name` is the function's name.
    void update_name_(Function* function, const char* name, SEXP r_rho) {
        // NOTE: A function's lexical environment is a namespace. If r_rho is a
        // package environment, we retrieve the corresponding namespace by
        // querying the namespace registry with the package name (without the
        // "package:" prefix)
        if (R_IsPackageEnv(r_rho)) {
            r_rho = infer_namespace_(r_rho);
        }

        SEXP r_lexenv = CLOENV(function->get_op());

        // function has a name in its lexical env
        if (r_lexenv == r_rho) {
            function->set_name(name);
        }
    }
};

#endif // VTRACE_FUNCTION_TABLE_H
