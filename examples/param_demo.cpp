#include "sqp/qp.hpp"
#include <iostream>

int main(){
    QP::Variable x(3);
    QP::Variable y(2);

    QP::Parameter p(2,'P');

    QP::QP_Params qp_params;
    qp_params.vars = {&x,&y};
    qp_params.params = {&p};

    QP qp(qp_params);

    ROW row = x[0];

    std::cout << "row: " ;
    for (auto row_entry : row.entries){
        std::cout << "<" << row_entry.first << " , " << row_entry.second << "> , ";
    }
    std::cout << std::endl;

    std::cout << "var_indices: ";
    for (auto var_index : p.var_indices){
        std::cout << var_index << " , ";
    }
    std::cout << std::endl;

    std::cout << "constr_indices: ";
    for (auto constr_index : p.constr_indices){
        std::cout << constr_index << " , ";
    }
    std::cout << std::endl;

    p.data[0] = 11;
    p.data[1] = 22;
    row = p[1]*x[0] + p[0]*y[1];

    std::cout << "row: " ;
    for (auto row_entry : row.entries){
        std::cout << "<" << row_entry.first << " , " << row_entry.second << "> , ";
    }
    std::cout << std::endl;


    std::cout << "var_indices: ";
    for (auto var_index : p.var_indices){
        std::cout << var_index << " , ";
    }
    std::cout << std::endl;

    std::cout << "constr_indices: ";
    for (auto constr_index : p.constr_indices){
        std::cout << constr_index << " , ";
    }
    std::cout << std::endl;

    
    return 0;
}