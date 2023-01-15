#include "sqp/qp.hpp"
#include <iostream>

int main(){
    QP::Variable x(3);
    QP::Parameter p(3,'A');
    QP::Parameter lb(1,'l');
    QP::Parameter ub(1,'u');

    QP::QP_Params qp_params;
    qp_params.vars = {&x};
    qp_params.params = {&p,&lb,&ub};
    QP qp(qp_params);

    qp.add_constraint(99,  p[1]*x[0] + p[1]*x[1] + p[2]*x[2], ub[0]);
    qp.add_constraint(100, 20*x[0] + p[1]*x[1] + p[2]*x[2], ub[0]);
    
    qp.formulate();

    p.update({-11,-22,-33});
    lb.update({-222});
    ub.update({-444});
    qp.update();
    
}