#include "sqp/qp.hpp"
#include <iostream>

int main(){
    QP::Variable x(3);
    QP::Parameter p(3);

    QP::QP_Params qp_params;
    qp_params.vars = {&x};
    qp_params.params = {&p};
    QP qp(qp_params);

    qp.add_constraint(0 , x[0] + x[1] + x[2] , 1);
    qp.add_constraint(p[0], p[0]*x[0] - p[1]*x[1] + p[2]*x[2], p[2]);
    qp.formulate();

    p.update_data({-11,-22,-33});
    qp.update();

    
}