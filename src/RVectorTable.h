#ifndef VTRACE_RVECTOR_TABLE_H
#define VTRACE_RVECTOR_TABLE_H

#include <R.h>
#include <Rinternals.h>
#include <fstream>
#include <unordered_map>

#include "RVector.h"

class RVectorTable {
  public:
    RVectorTable() = default;

    ~RVectorTable() {
        for (auto& it : table_) {
            delete it.second;
        }
    }

    void insert(RVector* vec) {
        table_.insert({vec->get_id(), vec});
        addr_to_id_.insert({vec->get_address(), vec->get_id()});
    }

    void remove(int id) {
        auto result = table_.find(id);
        if (result != table_.end()) {
            RVector* vec = result->second;
            table_.erase(result);
            auto res2 = addr_to_id_.find(vec->get_address());
            if (res2 != addr_to_id_.end()) {
                addr_to_id_.erase(res2);
            }
            delete vec;

        }
    }

    RVector* lookup(int id) {
        auto result = table_.find(id);
        if (result != table_.end()) {
            return result->second;
        } else {
            return nullptr;
        }
    }

    RVector* lookup_by_addr(std::string addr) {
        auto result = addr_to_id_.find(addr);
        if (result != addr_to_id_.end()) {
            return lookup(result->second);
        } else {
            return nullptr;
        }
    }

    RVector* get_copy_of(int id) {
        RVector* cur = lookup(id);
        if (cur && cur->is_copy()) {
            return lookup(cur->get_copy_of());
        }
        return nullptr;
    }

    RVector* get_original_copy_of(int id) {
        RVector* cur = lookup(id);
        while (cur) {
            if (!cur->is_copy()) {
                break;
            }

            cur = lookup(cur->get_copy_of());
        }
        return cur;
    }

    void dump_table_to_csv(std::ofstream& file) {
        file << "id,addr,type,length,copy_of,original\n";
        for (const auto& it: table_) {
            auto vec = it.second;
            auto id = vec->get_id();
            std::string copy_of = vec->is_copy() ? std::to_string(get_copy_of(id)->get_id()) : "NA";
            std::string original = vec->is_copy() ? std::to_string(get_original_copy_of(id)->get_id()) : "NA";
            file << vec->get_id() << "," << vec->get_address() << ","
                 << type2char(vec->get_type()) << ","
                 << vec->get_length() << "," << copy_of << "," << original << "\n";
        }
    }

  private:
    std::unordered_map<int, RVector*> table_;
    std::unordered_map<std::string, int> addr_to_id_;
};



#endif // VTRACE_RVECTOR_TABLE_H
