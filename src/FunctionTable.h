#ifndef VTRACE_FUNCTION_TABLE_H
#define VTRACE_FUNCTION_TABLE_H

#include <R.h>
#include <Rinternals.h>
#include "Function.h"

class FunctionTable {
  public:
    FunctionTable() {
        update_packages();
    }

    ~FunctionTable() {
        for (auto& it: table_) {
            delete it.second;
        }
    }

    void insert(SEXP r_closure) {
        Function* function = new Function(r_closure);

        auto result = table_.insert({r_closure, function});

        if (!result.second) {
            delete result.first->second;
            result.first->second = function;
        }
    }

    void remove(SEXP r_closure) {
        auto result = table_.find(r_closure);

        if (result != table_.end()) {
            Function* function = result->second;
            table_.erase(result);
            delete function;
        }
    }

    Function* lookup(SEXP r_closure) {
        return get_or_create_(r_closure);
    }

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
        // function->get_definition();
        // function->get_hash();
    }

    void update_packages() {
        SEXP package_names = R_lsInternal(R_NamespaceRegistry, TRUE);
        for (int i = 0; i < Rf_length(package_names); ++i) {
            const char* pkg_name = CHAR(STRING_ELT(package_names, i));
            SEXP r_obj =
                Rf_findVarInFrame(R_NamespaceRegistry, Rf_install(pkg_name));

            if (TYPEOF(r_obj) != ENVSXP) {
                continue;
            }
            if (seen_packages_.find(r_obj) != seen_packages_.end()) {
                continue;
            } else {
                seen_packages_.insert(r_obj);
            }

            SEXP r_names = R_lsInternal(r_obj, TRUE);
            for (int i = 0; i < Rf_length(r_names); ++i) {
                const char* name = CHAR(STRING_ELT(r_names, i));
                SEXP r_fun = Rf_findVarInFrame(r_obj, Rf_install(name));
                update(r_fun, name, r_obj);
            }
        }
    }

    void dump_table_to_csv(std::ofstream& file) {
        file << "hash,definition\n";
        for (const auto& it: table_) {
            auto fun = it.second;
            if (fun->is_called()) {
                file << fun->get_hash() << "," << fun->get_definition() << "\n";
            }
        }
    }

  private:
    std::unordered_map<SEXP, Function*> table_;
    std::unordered_set<SEXP> seen_packages_;

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
                if (r_closure == R_UnboundValue ||
                    TYPEOF(r_closure) != CLOSXP) {
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
        }

        else if (r_package == R_GlobalEnv) {
            return R_GlobalEnv;
        }

        else if (R_IsNamespaceEnv(r_package)) {
            return r_package;
        }

        else if (R_IsPackageEnv(r_package)) {
            const char* package_name =
                CHAR(STRING_ELT(R_PackageEnvName(r_package), 0));
            return Rf_findVarInFrame(
                R_NamespaceRegistry,
                Rf_install(package_name + strlen("package:")));
        }

        else {
            return r_package;
        }
    }

    Function* get_or_create_(SEXP r_closure) {
        auto result = table_.find(r_closure);

        if (result != table_.end()) {
            return result->second;
        } else {
            Function* function = new Function(r_closure);
            auto result = table_.insert({r_closure, function});
            return function;
        }
    }

    Function* update_name_(Function* function, const char* name, SEXP r_rho) {
        /* NOTE: A function's lexical environment is a namespace. If r_rho is a
         * package environment, we retrieve the corresponding namespace by
         * querying the namespace registry with the package name (without the
         * "package:" prefix)*/
        if (R_IsPackageEnv(r_rho)) {
            r_rho = infer_namespace_(r_rho);
        }

        SEXP r_lexenv = CLOENV(function->get_op());

        /* function has a name in its lexical env */
        if (r_lexenv == r_rho) {
            function->set_name(name);
        }

        return function;
    }
};

#endif /* VTRACE_FUNCTION_TABLE_H */
