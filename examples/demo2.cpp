#include "osqp-interface/qp.hpp"
#include <iostream>

int main(){
    // set variables and parameters
    QP::Variable x(1),y(1);
    QP::Parameter p(2);
    p.set_data({1,2});

    // create problem instance
    QP::QP_Params qp_param;
    qp_param.vars = {&x,&y};
    qp_param.params = {&p};
    QP qp(qp_param);

    // add objective function
    qp.add_cost(p[0]*x[0] + p[1]*y[0]);

    // add constraints
    qp.add_constraint(QP::NEG_INF, x[0] + y[0], 1);
    qp.add_constraint(-1, x[0] - y[0], QP::POS_INF);
    qp.add_constraint(0, y[0], QP::POS_INF);

    // formulate and solve
    qp.formulate();
    qp.solve();

    // print out results
    std::cout << "Status: " << qp.get_status() << std::endl;
    std::cout << "Objective Value: " << qp.get_obj_val() << std::endl;
    std::cout << "Solution: x = " << x.solution[0] << " , y = " << y.solution[0] << std::endl;

    // update parameter and solve
    p.set_data({-1,0});
    qp.update();

    // solve
    qp.solve();

    // print out results
    std::cout << "Status: " << qp.get_status() << std::endl;
    std::cout << "Objective Value: " << qp.get_obj_val() << std::endl;
    std::cout << "Solution: x = " << x.solution[0] << " , y = " << y.solution[0] << std::endl;

    return 0;
}