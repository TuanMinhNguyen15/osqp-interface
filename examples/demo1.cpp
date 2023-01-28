#include "osqp-interface/qp.hpp"
#include <iostream>

int main(){
    // create variables
    QP::Variable x(2);

    // setup problem instance
    QP::QP_Params qp_params;
    qp_params.vars = {&x};
    QP qp(qp_params);

    // add objective function
    qp.add_cost(2*x[0]*x[0] + x[1]*x[1] + x[0]*x[1] + x[0] + x[1]); 

    // add constraints
    qp.add_constraint(1, x[0] + x[1], 1);
    qp.add_constraint(0, x[0], 0.7);
    qp.add_constraint(0, x[1], 0.7);

    // formulate and solve
    qp.formulate();
    
    qp.solve();

    // print out results
    std::cout << "Status: " << qp.get_status() << std::endl;
    std::cout << "Objective Value: " << qp.get_obj_val() << std::endl;
    std::cout << "Solution: ";
    for (auto sol : x.solution){
        std::cout << sol << " , ";
    }
    std::cout << std::endl;

}