#include <iostream>
#include "sqp/util.hpp"

int main(){
    c_float *M_x;
    c_int M_nnz;
    c_int *M_i;
    c_int *M_p;

    size_t num_cols = 3;
    CSC_GEN M(num_cols);
    ROW row1,row2;

    row1.entries = {{0,11},{2,22}};
    row2.entries = {{0,33}};

    M.add_row(row1);
    M.add_row(row2);

    M.csc_generate(M_x,M_nnz,M_i,M_p);

    std::cout << "M_nnz: " << M_nnz << std::endl;

    std::cout << "M_x: ";
    for (int i = 0; i < M_nnz; i++){
        std::cout << M_x[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "M_i: ";
    for (int i = 0; i < M_nnz; i++){
        std::cout << M_i[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "M_p: ";
    for (int i = 0; i < num_cols+1; i++){
        std::cout << M_p[i] << " ";
    }
    std::cout << std::endl;

    return 0;
}