#include <iostream>
#include "sqp/util.hpp"

int main(){
    c_float *M_x;
    c_int M_nnz;
    c_int *M_i;
    c_int *M_p;

    size_t num_cols = 2;
    CSC_GEN M(num_cols);
    ROW_COL rc1,rc2,rc3,rc4,rc;

    rc1.entries.insert({{0,0},1});
    rc2.entries.insert({{1,1},1});
    rc3.entries.insert({{1,0},1});
    rc4.entries.insert({{0,0},9});
    rc = rc1 - rc2 + rc3 - rc4;

    M.add_row_col(rc);
    M.csc_generate(M_x,M_nnz,M_i,M_p);
    M.print_matrix();
    

    return 0;
}