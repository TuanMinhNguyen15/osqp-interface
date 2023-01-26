# C++ Interface for OSQP solver

The purpose of this project is to create an intuitive c++ interface for the osqp solver. Inspired by the convenience of cvxpy, the interface supports both high-level problem formulation and parameterization.

## Example Usage - Problem Formulation
The code below formulates and solves the following 2-dimensional optimization problem:

```
 min    2*x0^2 + x1^2 + x0*x1 + x0 + x1
x0,x1

subject to      0 <= x0 + x1 <= 1
                0 <= x0 <= 0.7
                0 <= x1 <= 0.7     
```

```C++
// create variables
QP::Variable x(2);

// setup problem instance
QP::QP_Params qp_params;
qp_params.vars = {&x};
QP qp(qp_params);
qp.get_settings()->verbose = 0; 

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
```

Terminal output:
```
Status: solved
Objective Value: 1.87975
Solution: 0.298771 , 0.701228 ,
```