#pragma once

#include "osqp.h"
#include <vector>
#include <utility>
#include <map>
#include <iostream>

struct ROW{
    // {{index0,data0} , {index1,data1} , {index2,data2} ,}
    std::map<int,c_float> entries;
    ROW(){
        entries.clear();
    }
};

ROW operator + (ROW row_a, ROW row_b);
ROW operator - (ROW row_a, ROW row_b);
ROW operator * (float coeff, ROW row);

struct SYM_M {
    // matrix_info
    // [col0 , col1 , col2 ...]
    // [<row,data> , s<row,data> , <row,data> ... ]
    std::map<int,ROW> matrix_info;

    SYM_M(int dim){
        matrix_info.clear();
        for (int i = 0; i < dim; i++){
            matrix_info.insert({i,ROW()});
        }
    }
};

class CSC_GEN{
    public:
        CSC_GEN(int num_cols);
        CSC_GEN();

        void add_row(ROW row);
        void update_num_cols(int num_cols);
        void csc_generate(c_float *&M_x, c_int &M_nnz, c_int *&M_i, c_int *&M_p);
        void print_matrix();
        void restore();

    private:
        // num_cols <=> number of variables
        // num_row  <=> number of constraints
        int num_cols;
        int num_rows = 0;

        // matrix_info
        // [col0 , col1 , col2 ...]
        // [<row,data> , <row,data> , <row,data> ... ]
        std::vector<std::vector<std::pair<int,c_float>>> matrix_info;

        // data 
        std::vector<c_float> csc_x,csc_x_original;

        // number of non-zeros
        c_int csc_nnz = 0;

        // row indices
        std::vector<c_int> csc_i;

        // column pointers
        std::vector<c_int> csc_p;
};