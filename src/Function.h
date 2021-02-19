#ifndef VTRACE_FUNCTION_H
#define VTRACE_FUNCTION_H

#include <string>

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

  private:
    SEXP r_op_;
    SEXPTYPE type_;
    std::string name_;
    std::string package_name_;
};

#endif /* VTRACE_FUNCTION_H */
