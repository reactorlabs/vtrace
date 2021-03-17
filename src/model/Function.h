#ifndef VTRACE_FUNCTION_H
#define VTRACE_FUNCTION_H

#include <R.h>
#include <Rinternals.h>
#include <Rdyntrace.h>
#include <string>

#include "../lib/picosha2.h"

static constexpr char NotComputed[] = "<not computed>";
static constexpr int FUN_DEF_MAX = 60;

/*
 * This class models R function objects, which include builtin functions,
 * special functions, and closures.
 *
 * The function's definition (a string containing the first FUN_DEF_MAX lines
 * of the function's R source code) and hash are computed only if the function
 * has been called.
 *
 * The hash is just a convenient way of summarizing the function definition; we
 * use SHA256 hex string.
 *
 * NOTE: Only the FunctionTable should be constructing Functions.
 */
class Function {
    static int counter;

    int id_;
    SEXP r_op_;
    SEXPTYPE type_;
    std::string name_;
    std::string package_name_;
    std::string definition_ = NotComputed;
    std::string hash_ = NotComputed;
    int called_ = 0;
    bool finalized_ = false;

  public:
    explicit Function(SEXP r_op)
        : id_(counter++)
        , r_op_(r_op)
        , type_(TYPEOF(r_op)) {

        // Try to initialize the package name and function name
        if (type_ == BUILTINSXP || type_ == SPECIALSXP) {
            package_name_ = "base";
            name_ = dyntrace_get_c_function_name(r_op);
        } else {
            // In this case, we'll compute the function name later
            SEXP r_lexenv = CLOENV(r_op);

            if (r_lexenv == R_GlobalEnv) {
                package_name_ = "global";
            } else if (r_lexenv == R_BaseEnv || r_lexenv == R_BaseNamespace) {
                package_name_ = "base";
            } else if (R_IsPackageEnv(r_lexenv)) {
                package_name_ =
                    CHAR(STRING_ELT(R_PackageEnvName(r_lexenv), 0));
            } else if (R_IsNamespaceEnv(r_lexenv)) {
                package_name_ =
                    CHAR(STRING_ELT(R_NamespaceEnvSpec(r_lexenv), 0));
            }
        }
    }

    int get_id() const {
        return id_;
    }

    SEXP get_op() const {
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

    // Qualified name is: `<package>::<name>` or `<name>` if there is no package
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

    // Function has been called, so initialize its definition and hash
    void called() {
        set_hash_();
        ++called_;
    }

    bool is_called() const {
        return called_;
    }

    void finalize() {
        finalized_ = true;
    }

  private:
    const std::string& set_hash_() {
        if (hash_ == NotComputed) {
            hash_ = picosha2::hash256_hex_string(set_definition_());
        }
        return hash_;
    }

    // TODO: some function is too long so we trim to FUN_DEF_MAX lines, which one is it?
    const std::string& set_definition_() {
        if (definition_ == NotComputed) {
            // Construct an R call to deparse the function, so we can get its
            // definition as a string
            SEXP deparse_call =
                Rf_lang3(Rf_install("deparse"), r_op_, ScalarInteger(FUN_DEF_MAX));
            SET_TAG(CDDR(deparse_call), Rf_install("nlines"));
            SEXP def = Rf_eval(deparse_call, R_BaseEnv);

            // def is an R vector, now we convert it to a C++ string
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

int Function::counter = 0;

#endif // VTRACE_FUNCTION_H
