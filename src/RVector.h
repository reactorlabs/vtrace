#ifndef VTRACE_RVECTOR_H
#define VTRACE_RVECTOR_H

#include <R.h>
#include <Rinternals.h>
#include <string>

static int vec_counter = 0;

class RVector {
  public:
    RVector(std::string addr, SEXPTYPE type, int length)
        : id_(vec_counter++)
        , address_(addr)
        , type_(type)
        , length_(length) { }

    int get_id() const {
        return id_;
    }

    const std::string& get_address() const {
        return address_;
    }

    SEXPTYPE get_type() const {
        return type_;
    }

    int get_length() const {
        return length_;
    }

    void finalize() {
        finalized = true;
    }

    void set_copy_of(int copy_of) {
        copy_of_ = copy_of;
    }

    int get_copy_of() const {
        return copy_of_;
    }

    bool is_copy() const {
        return copy_of_ != -1;
    }

  private:
    int id_;
    bool finalized = false;
    std::string address_;
    SEXPTYPE type_;
    int length_;
    int copy_of_ = -1;
};

#endif // VTRACE_RVECTOR_H
