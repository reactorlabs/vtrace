#ifndef VTRACE_FUNCTION_H
#define VTRACE_FUNCTION_H

#include <R.h>
#include <Rinternals.h>
#include <Rdyntrace.h>
#include <string>

#include "picosha2.h"

class Function {
  public:
    explicit Function(SEXP r_op): r_op_(r_op) {
        type_ = TYPEOF(r_op);

        if (type_ == BUILTINSXP || type_ == SPECIALSXP) {
            package_name_ = "base";
            name_ = dyntrace_get_c_function_name(r_op);
        } else {
            SEXP r_lexenv = CLOENV(r_op);

            if (r_lexenv == R_GlobalEnv) {
                package_name_ = "global";
            }

            else if (r_lexenv == R_BaseEnv || r_lexenv == R_BaseNamespace) {
                package_name_ = "base";
            }

            else if (R_IsPackageEnv(r_lexenv)) {
                package_name_ = CHAR(STRING_ELT(R_PackageEnvName(r_lexenv), 0));

            }

            else if (R_IsNamespaceEnv(r_lexenv)) {
                package_name_ =
                    CHAR(STRING_ELT(R_NamespaceEnvSpec(r_lexenv), 0));
            }
        }
    }

    SEXP get_op() {
        return r_op_;
    }

    SEXPTYPE get_type() const {
        return type_;
    }

    bool is_closure() const {
        return type_ == CLOSXP;
    }

    bool is_special() const {
        return type_ == SPECIALSXP;
    }

    bool is_builtin() const {
        return type_ == BUILTINSXP;
    }

    bool has_name() const {
        return !name_.empty();
    }

    const std::string& get_name() const {
        return name_;
    }

    void set_name(const char* name) {
        name_ = name;
    }

    bool has_package_name() const {
        return !package_name_.empty();
    }

    const std::string& get_package_name() const {
        return package_name_;
    }

    std::string get_qualified_name() const {
        std::string qualified;

        if (has_package_name()) {
            qualified.append(get_package_name());
            qualified.append("::");
        }

        if (has_name()) {
            qualified.append(get_name());
        } else {
            qualified.append("<unknown>");
        }

        return qualified;
    }

    std::string get_definition() {
        if (definition_.empty()) {
            definition_ = serialize_r_expression(r_op_);
        }
        return definition_;
    }

    std::string get_hash() {
        if (hash_.empty()) {
            hash_ = set_hash();
        }
        return hash_;
    }

    void finalize() {
        finalized = true;
    }

  private:
    bool finalized = false;
    SEXP r_op_;
    SEXPTYPE type_;
    std::string name_;
    std::string package_name_;
    std::string definition_;
    std::string hash_;

    std::string set_hash() {
        if (finalized) {
            return "<finalized>";
        }
        return picosha2::hash256_hex_string(get_definition());
    }

    std::string serialize_r_expression(SEXP e) {
        if (finalized) {
            return "<finalized>";
        }
        SEXP deparse_call = Rf_lang3(Rf_install("deparse"), r_op_, ScalarInteger(60));
        SET_TAG(CDDR(deparse_call), Rf_install("nlines"));
        SEXP def = Rf_eval(deparse_call, R_BaseEnv);
        std::string to_ret = "";
        for (int i = 0; i < Rf_length(def); ++i) {
            auto name = STRING_ELT(def, i);
            if (name == NA_STRING) {
                to_ret.append("NA");
            } else {
                to_ret.append(CHAR(name));
                to_ret.append("\n");
            }
        }
        return to_ret;
    }
};

#endif /* VTRACE_FUNCTION_H */
