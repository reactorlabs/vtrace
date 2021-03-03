#ifndef VTRACE_FUNCTION_H
#define VTRACE_FUNCTION_H

#include <R.h>
#include <Rinternals.h>
#include <Rdyntrace.h>
#include <string>

#include "picosha2.h"

static const std::string NotComputed = "<not computed>";
static int counter = 0;

class Function {
  public:
    explicit Function(SEXP r_op)
        : id_(counter++)
        , r_op_(r_op)
        , definition_(NotComputed)
        , hash_(NotComputed)
        , called_(0) {
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

    int get_id() {
        return id_;
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

    const std::string& get_definition() const {
        return definition_;
    }

    const std::string& get_hash() const {
        return hash_;
    }

    void called() {
        set_definition_();
        set_hash_();
        ++called_;
    }

    bool is_called() const {
        return called_;
    }

    void finalize() {
        finalized = true;
    }

  private:
    int id_;
    bool finalized = false;
    SEXP r_op_;
    SEXPTYPE type_;
    std::string name_;
    std::string package_name_;
    std::string definition_;
    std::string hash_;
    int called_;

    const std::string& set_hash_() {
        if (hash_ == NotComputed) {
            hash_ = picosha2::hash256_hex_string(set_definition_());
        }

        return hash_;
    }

    const std::string& set_definition_() {
        if (definition_ == NotComputed) {
            SEXP deparse_call =
                Rf_lang3(Rf_install("deparse"), r_op_, ScalarInteger(60));
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
            definition_ = to_ret;
        }
        return definition_;
    }
};

#endif /* VTRACE_FUNCTION_H */
