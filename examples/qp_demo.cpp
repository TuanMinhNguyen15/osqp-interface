#include "osqp-interface/qp.hpp"
#include <iostream>

int main(){
    QP::Variable x(2);

    QP::QP_Params qp_params;
    qp_params.vars = {&x};
    QP qp(qp_params);
    qp.get_settings()->verbose = 0; 

    qp.add_cost(2*x[0]*x[0] + x[1]*x[1] + x[0]*x[1] + x[0] + x[1]); 

    qp.add_constraint(1, x[0] + x[1], 1);
    qp.add_constraint(0, x[0], 0.7);
    qp.add_constraint(0, x[1], 0.7);

    qp.formulate();
    
    qp.solve();

    std::cout << "solution: ";
    for (auto sol : x.solution){
        std::cout << sol << " , ";
    }
    std::cout << std::endl;

    std::cout << "Objective Value: " << qp.get_obj_val() << std::endl;
    
    std::cout << "Status: " << qp.get_status() << std::endl;

}