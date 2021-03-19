#ifndef VTRACE_RVECTOR_H
#define VTRACE_RVECTOR_H

#include <R.h>
#include <Rinternals.h>
#include <sstream>
#include <string>

class Vector {
    static int counter;

    int id_;
    SEXP r_vec_;
    SEXPTYPE type_;
    int length_;
    std::string address_;
    Vector* copy_of_ = nullptr;
    bool finalized_ = false;

  public:
    explicit Vector(SEXP r_vec)
        : id_(counter++)
        , r_vec_(r_vec)
        , type_(TYPEOF(r_vec))
        , length_(Rf_length(r_vec)) {

        std::stringstream ss;
        ss << static_cast<const void*>(r_vec);
        address_ = ss.str();
    }

    int get_id() const {
        return id_;
    }

    SEXP get_vec() const {
        // NOTE: SEXP may be invalid if the vector has been finalized!
        return r_vec_;
    }

    SEXPTYPE get_type() const {
        return type_;
    }

    int get_length() const {
        return length_;
    }

    const std::string& get_address() const {
        return address_;
    }

    bool is_finalized() const {
        return finalized_;
    }

    void finalize() {
        finalized_ = true;
    }

    bool is_copy() const {
        return copy_of_ != nullptr;
    }

    void set_copy_of(Vector* copy_of) {
        copy_of_ = copy_of;
    }

    Vector* get_copy_of() const {
        return copy_of_;
    }

    Vector* get_original_copy_of() {
        Vector* cur = this;
        while (cur->is_copy()) {
            cur = cur->get_copy_of();
        }
        return cur;
    }
};

int Vector::counter = 0;

#endif // VTRACE_RVECTOR_H
