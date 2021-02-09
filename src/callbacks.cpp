#include <vector>
#include <fstream>
#include <iostream>
#include "Rinternals.h"
#include "callbacks.h"

std::vector<std::string> input_addr;
std::vector<std::string> output_addr;
std::vector<std::string> type;
std::vector<std::string> length;
char buffer[1024];

void object_duplicate_callback(ContextSPtr context,
                                 ApplicationSPtr application,
                                 SEXP r_input,
                                 SEXP r_output,
                                 SEXP r_deep) {

    auto t = TYPEOF(r_input);

    if (t == INTSXP || t == REALSXP || t == CPLXSXP || t == LGLSXP || t == RAWSXP || t == STRSXP || t == VECSXP) {
        sprintf(buffer, "%p", r_input);
        input_addr.push_back(std::string(buffer));

        sprintf(buffer, "%p", r_output);
        output_addr.push_back(std::string(buffer));

        type.push_back(std::string(type2char(TYPEOF(r_input))));

        length.push_back(std::to_string(Rf_length(r_input)));
    }

}

void application_unload_callback(ContextSPtr context,
                                    ApplicationSPtr application) {

    std::ofstream file("duplication.csv");

    file << "input_addr,output_addr,type,length\n";

    for (int i = 0; i < input_addr.size(); ++i) {
        file << input_addr[i] << "," << output_addr[i] << "," << type[i] << "," << length[i] << "\n";
    }

}
