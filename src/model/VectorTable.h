#ifndef VTRACE_RVECTOR_TABLE_H
#define VTRACE_RVECTOR_TABLE_H

#include <R.h>
#include <Rinternals.h>
#include <fstream>
#include <unordered_map>
#include <unordered_set>

#include "Vector.h"

/*
 * The VectorTable keeps track of all vectors we have seen. It is responsble
 * for allocating and deallocating Vector objects.
 *
 * The VectorTable uses map to track _live_ vectors, and a set to track
 * _finalized_ vectors. This is because an address can be allocated to only one
 * vector at a time, but addresses can be reused if a vector is finalized.
 *
 * Currently, the VectorTable does not track R vector deallocation: if an
 * address is reused, we assume that the vector previously allocated to that
 * address has been finalized.
 */
class VectorTable {
  private:
    // Map of SEXP to (live) tracer Vector pointers
    std::unordered_map<SEXP, Vector*> table_;
    // Set of (finalized) tracer Vector pointers
    std::unordered_set<Vector*> finalized_;

  public:
    VectorTable() = default;

    // Delete all the Vector objects in the table
    ~VectorTable() {
        for (auto& it : table_) {
            delete it.second;
        }
        for (auto& it : finalized_) {
            delete it;
        }
    }

    // Insert an SEXP into the table, creating a Vector object
    Vector* insert(SEXP r_vec) {
        Vector* vec = new Vector(r_vec);
        auto result = table_.emplace(r_vec, vec);

        if (!result.second) {
            // For now, assume the old vector was finalized
            //Rf_error("vector was not finalized before reusing address");
            finalized_.emplace(result.first->second);
            result.first->second = vec;
        }

        return vec;
    }

    void duplicate(SEXP r_input, SEXP r_output) {
        // For now, create the vector if we don't find it
        auto input = lookup(r_input);

        // Note that the object alloc callback is called before the object
        // duplicate callback, so we've already seen the vector
        auto output = lookup(r_output);
        output->set_copy_of(input);
    }

    void finalize(SEXP r_vec) {
        auto result = table_.find(r_vec);
        if (result != table_.end()) {
            Vector* vec = result->second;
            table_.erase(result);

            vec->finalize();
            finalized_.emplace(vec);
        } else {
            // WARN: causes recursive GC invocation
            //Rf_error("unknown vector that is being finalized");
        }
    }

    Vector* lookup(SEXP r_vec) {
        return get_or_create_(r_vec);
    }

    void dump_table_to_csv(std::ofstream& file) {
        auto dump_entry = [&file](Vector * vec) {
            std::string copy_of = vec->is_copy() ? std::to_string(vec->get_copy_of()->get_id()) : "NA";
            std::string original = vec->is_copy() ? std::to_string(vec->get_original_copy_of()->get_id()) : "NA";
            file << vec->get_id() << "," << vec->get_address() << ","
                 << type2char(vec->get_type()) << ","
                 << vec->get_length() << "," << copy_of << ","
                 << original << "\n";
        };

        file << "id,addr,type,length,copy_of,original\n";
        for (const auto& it : table_) {
            dump_entry(it.second);
        }
        for (const auto& it : finalized_) {
            dump_entry(it);
        }
    }

  private:
    Vector* get_or_create_(SEXP r_vec) {
        auto result = table_.find(r_vec);
        if (result != table_.end()) {
            return result->second;
        } else {
            Vector* vec = new Vector(r_vec);
            table_.emplace(r_vec, vec);
            return vec;
        }
    }
};

#endif // VTRACE_VECTOR_TABLE_H
