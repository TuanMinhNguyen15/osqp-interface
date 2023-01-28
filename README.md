# C++ Interface for OSQP solver

The purpose of this project is to create an intuitive c++ interface for the [osqp](https://osqp.org/docs/index.html) solver. Inspired by the convenience of [cvxpy](https://www.cvxpy.org/), the interface supports both high-level problem formulation and parameterization.

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
Objective Value: 1.88
Solution: 0.3 , 0.7 ,
```

## Example Ussage - Parameterization
The below code introduces parameterization into problem formulation in which the objective function is parametrized by p0 and p1. 

```
 min    p0*x + p1*y
 x,y

subject to  x + y <= 1
            x - y >= -1
            y >= 0   
```

Initially, let p0 =1 and p1 = 2.

```C++
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

```

Terminal output:
```
Status: solved
Objective Value: -0.997452
Solution: x = -0.998908 , y = 0.000728062
```

Now, update p0 = -1 and p1 = 0

```C++
// update parameter and solve
p.set_data({-1,0});
qp.update();

// solve
qp.solve();

// print out results
std::cout << "Status: " << qp.get_status() << std::endl;
std::cout << "Objective Value: " << qp.get_obj_val() << std::endl;
std::cout << "Solution: x = " << x.solution[0] << " , y = " << y.solution[0] << std::endl;


```

Terminal output:
```
Status: solved
Objective Value: -1.00081
Solution: x = 1.00081 , y = -0.000540838
```