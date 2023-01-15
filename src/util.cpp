#include "SQP/util.hpp"

CSC_GEN::CSC_GEN(int num_cols):num_cols(num_cols){
    matrix_info.clear();
    csc_x.clear();
    csc_i.clear();
    csc_p.clear();

    matrix_info.resize(num_cols);
}

CSC_GEN::CSC_GEN(){
    matrix_info.clear();
    csc_x.clear();
    csc_i.clear();
    csc_p.clear();
}

void CSC_GEN::add_row(ROW row){
    // row
    // [<col,data> , <col,data> , <col,data> ...]

    for (auto entry : row.entries){
        matrix_info[entry.first].push_back({num_rows,entry.second});
        csc_nnz++;
    }

    num_rows++;
}

void CSC_GEN::update_num_cols(int num_cols){
    this->num_cols = num_cols;
    matrix_info.resize(num_cols);
}

void CSC_GEN::csc_generate(c_float *&M_x, c_int &M_nnz, c_int *&M_i, c_int *&M_p){
    csc_p.push_back(0);
    for (auto column : matrix_info){
        for (auto row : column){
            csc_x.push_back(row.second); // data
            csc_i.push_back(row.first);  // row index
        }
        csc_p.push_back(csc_p.back() + column.size()); // column pointer
    }

    M_x   = &csc_x[0];
    M_nnz = csc_nnz;
    M_i   = &csc_i[0];
    M_p   = &csc_p[0];
}

void CSC_GEN::print_matrix(){
    std::cout << "M_nnz: " << csc_nnz << std::endl;

    std::cout << "M_x: ";
    for (int i = 0; i < csc_nnz; i++){
        std::cout << csc_x[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "M_i: ";
    for (int i = 0; i < csc_nnz; i++){
        std::cout << csc_i[i] << " ";
    }
    std::cout << std::endl;

    std::cout << "M_p: ";
    for (int i = 0; i < num_cols+1; i++){
        std::cout << csc_p[i] << " ";
    }
    std::cout << std::endl;
}


ROW operator + (ROW row_a, ROW row_b){

    for (auto row_b_entry : row_b.entries){
        auto found = row_a.entries.find(row_b_entry.first);
        if (found != row_a.entries.end()){
            // Found
            row_a.entries[row_b_entry.first] += row_b_entry.second;
        }
        else{
            // Not Found
            row_a.entries.insert(row_b_entry);
        }
    }

    return row_a;
}

ROW operator - (ROW row_a, ROW row_b){

    for (auto row_b_entry : row_b.entries){
        auto found = row_a.entries.find(row_b_entry.first);
        if (found != row_a.entries.end()){
            // Found
            row_a.entries[row_b_entry.first] -= row_b_entry.second;
        }
        else{
            // Not Found
            row_b_entry.second *= -1;
            row_a.entries.insert(row_b_entry);
        }
    }

    return row_a;
}

ROW operator * (float coeff, ROW row){

    for (auto &row_entry : row.entries){
        row_entry.second *= coeff;
    }

    return row;
}