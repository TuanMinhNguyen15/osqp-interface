#include "sqp/qp.hpp"

// ---------- Variable ----------

QP::Variable::Variable(int size):size(size){
     sol.resize(size);
}

Expression QP::Variable::operator[] (int index){
     Expression exp;
     exp.linear_terms.entries.clear();
     exp.linear_terms.entries.insert({col_start+index,1});

     return exp;
}

// ---------- Parameter ----------

QP::Parameter::Parameter(int size){
     this->size = size;
     data.resize(size,0);
     types.resize(size);
     var_indices.resize(size);
     constr_indices.resize(size);
     targets.resize(size);
}

void QP::Parameter::update_data(std::vector<c_float> data_new){
     if (data_new.size() == size){
          data = data_new;
     }
     else{
          std::cerr << "Invalid size of new data\n";
     }
}

PARAM_DUMMY QP::Parameter::operator[] (int index){
     PARAM_DUMMY param_dummy;
     param_dummy.param_ptr = this;
     param_dummy.index = index;

     return param_dummy;
  
}

ROW operator * (PARAM_DUMMY param_dummy, ROW &row){
     // TODO: * operator only works on type 'A' parameters
     if (row.entries.size() == 1){
          auto entry_ptr = row.entries.begin(); 
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);

          entry_ptr->second = 0;
          param_ptr->var_indices[param_dummy.index].push_back(entry_ptr->first);
          param_ptr->constr_indices[param_dummy.index].push_back(param_ptr->qp->get_num_constr());
          
     }
     else{
          std::cerr << "row must have only one element!\n";
     }

     return row;
}

// ---------- Expression ----------
Expression::Expression(){
     param_dummies.clear();
}

Expression operator + (Expression exp1, Expression exp2){
     Expression exp_out;
     
     // Linear terms
     exp_out.linear_terms = exp1.linear_terms + exp2.linear_terms;

     // Quadratic terms

     // Params
     exp_out.param_dummies.insert(exp_out.param_dummies.end(),exp1.param_dummies.begin(),exp1.param_dummies.end());
     exp_out.param_dummies.insert(exp_out.param_dummies.end(),exp2.param_dummies.begin(),exp2.param_dummies.end());

     return exp_out;
}

Expression operator - (Expression exp1, Expression exp2){
     Expression exp_out;
     
     // Linear terms
     exp_out.linear_terms = exp1.linear_terms - exp2.linear_terms;

     // Quadratic terms

     // Params
     exp_out.param_dummies.insert(exp_out.param_dummies.end(),exp1.param_dummies.begin(),exp1.param_dummies.end());
     exp_out.param_dummies.insert(exp_out.param_dummies.end(),exp2.param_dummies.begin(),exp2.param_dummies.end());

     return exp_out;
}

Expression operator * (float coef, Expression exp){
     Expression exp_out;

     // Linear terms
     exp_out.linear_terms = coef*exp.linear_terms;

     return exp_out;
}

Expression operator * (PARAM_DUMMY param_dummy, Expression &exp){

     auto entry_ptr = exp.linear_terms.entries.begin();  
     auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);

     entry_ptr->second = 0; // param_dummy.param_data

     param_dummy.var_index = entry_ptr->first;
   
     exp.param_dummies.push_back(param_dummy);
     return exp;
}


// ---------- QP ----------

QP::QP(QP_Params qp_params):qp_params_(qp_params){
     for (auto var : qp_params_.vars){
       if (num_vars == 0){
            var->col_start = 0;  
       }
       else{
            var->col_start = num_vars;
       }
       num_vars += var->size;
     }

     for (auto param : qp_params_.params){
          param->qp = this;
     }

     A_matrix.update_num_cols(num_vars);
     P_matrix.update_num_cols(num_vars);
}

void QP::add_constraint(c_float lb, Expression exp, c_float ub){
     l_vec.push_back(lb);
     A_matrix.add_row(exp.linear_terms);
     u_vec.push_back(ub);

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          param_ptr->types[param_dummy.index].push_back(TYPE::CONSTRAINT);
          param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index); 
          param_ptr->constr_indices[param_dummy.index].push_back(num_constr);
     }

     num_constr++;
}

void QP::add_constraint(PARAM_DUMMY lb, Expression exp, c_float ub){
     l_vec.push_back(0); // lb.param_data
     A_matrix.add_row(exp.linear_terms);
     u_vec.push_back(ub);

     QP::Parameter *param_ptr = static_cast<QP::Parameter*>(lb.param_ptr);
     param_ptr->constr_indices[lb.index].push_back(num_constr);
     param_ptr->var_indices[lb.index].push_back(-1);
     param_ptr->types[lb.index].push_back(TYPE::LOWER_BOUND);

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          param_ptr->types[param_dummy.index].push_back(TYPE::CONSTRAINT);
          param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index); 
          param_ptr->constr_indices[param_dummy.index].push_back(num_constr);
     }

     num_constr++;
}

void QP::add_constraint(c_float lb, Expression exp, PARAM_DUMMY ub){
     l_vec.push_back(lb);
     A_matrix.add_row(exp.linear_terms);
     u_vec.push_back(0);  // ub.param_data
   

     QP::Parameter *param_ptr = static_cast<QP::Parameter*>(ub.param_ptr);
     param_ptr->constr_indices[ub.index].push_back(num_constr);
     param_ptr->var_indices[ub.index].push_back(-1);
     param_ptr->types[ub.index].push_back(TYPE::UPPER_BOUND);

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          param_ptr->types[param_dummy.index].push_back(TYPE::CONSTRAINT);
          param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index); 
          param_ptr->constr_indices[param_dummy.index].push_back(num_constr);
     }

     num_constr++;
}

void QP::add_constraint(PARAM_DUMMY lb, Expression exp, PARAM_DUMMY ub){
     l_vec.push_back(0); // lb.param_data
     A_matrix.add_row(exp.linear_terms);
     u_vec.push_back(0); // ub.param_data

     QP::Parameter *lb_param_ptr = static_cast<QP::Parameter*>(lb.param_ptr);
     lb_param_ptr->constr_indices[lb.index].push_back(num_constr);
     lb_param_ptr->var_indices[lb.index].push_back(-1);
     lb_param_ptr->types[lb.index].push_back(TYPE::LOWER_BOUND);

     QP::Parameter *ub_param_ptr = static_cast<QP::Parameter*>(ub.param_ptr);
     ub_param_ptr->constr_indices[ub.index].push_back(num_constr);
     ub_param_ptr->var_indices[ub.index].push_back(-1);
     ub_param_ptr->types[ub.index].push_back(TYPE::UPPER_BOUND);

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          param_ptr->types[param_dummy.index].push_back(TYPE::CONSTRAINT);
          param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index); 
          param_ptr->constr_indices[param_dummy.index].push_back(num_constr);
     }

     num_constr++;
}

void QP::formulate(){
     A_matrix.csc_generate(A_x,A_nnz,A_i,A_p);
     l = &l_vec[0];
     u = &u_vec[0];

     for (auto param : qp_params_.params){

          for (int i = 0; i < param->size; i++){
               param->targets[i].resize(param->types[i].size());
               for (int j = 0; j < param->types[i].size(); j++){
                    switch (param->types[i][j]){
                         case TYPE::LOWER_BOUND:
                              std::cout << "Lower bound detected\n";
                              param->targets[i][j].first  = l;
                              param->targets[i][j].second = param->constr_indices[i][j];
                              break;

                         case TYPE::UPPER_BOUND:
                              std::cout << "Upper bound detected\n";
                              param->targets[i][j].first  = u;
                              param->targets[i][j].second = param->constr_indices[i][j];
                              break;

                         case TYPE::CONSTRAINT:
                              std::cout << "Constraint detected \n";
                              param->targets[i][j].first  = A_x;
                              param->targets[i][j].second = A_p[param->var_indices[i][j]] + param->constr_indices[i][j];
                              break;

                         case TYPE::LINEAR_COST:
                              param->targets[i][j].first = q;
                              break;

                         case TYPE::QUADRATIC_COST:
                              param->targets[i][j].first = P_x;
                              break;

                         default:
                              std::cout << "oh no...\n";
                              break;
                    }
               }
          }

          // switch (param->type)
          // {
          // case 'P':
          //      param->target = P_x;
          //      break;

          // case 'q':
          //      param->target = q;
          //      break;

          // case 'l':
          //      param->target = l;
          //      for (auto i = 0; i < param->size; i++){
          //           param->target_indices[i] = param->constr_indices[i];
          //      }
          //      break;

          // case 'A':
          //      param->target = A_x;
          //      for (auto i = 0; i < param->size; i++){
          //           for (auto j = 0; j < param->var_indices[i].size() ; j++){
          //                param->target_indices[i].push_back(A_p[param->var_indices[i][j]] + param->constr_indices[i][j]);
          //           }
          //      }
          //      break;

          // case 'u':
          //      param->target = u;
          //      for (auto i = 0; i < param->size; i++){
          //           param->target_indices[i] = param->constr_indices[i];
          //      }
          //      break;
          
          // default:
          //      break;
          // }
     }

     A_matrix.print_matrix();

     std::cout << "l: ";
     for (int i = 0; i < num_constr; i++){
          std::cout << l[i] << " , ";
     }
     std::cout << std::endl;

     std::cout << "u: ";
     for (int i = 0; i < num_constr; i++){
          std::cout << u[i] << " , ";
     }
     std::cout << std::endl;
}

void QP::update(){
     A_matrix.restore();
     l_vec = l_original;
     u_vec = u_original;

     for (auto param : qp_params_.params){
          for (int data_index = 0; data_index < param->data.size(); data_index++){
               for (auto target : param->targets[data_index]){
                    target.first[target.second] += param->data[data_index];
               }
          }
     }

     A_matrix.print_matrix();

     std::cout << "l: ";
     for (int i = 0; i < num_constr; i++){
          std::cout << l[i] << " , ";
     }
     std::cout << std::endl;

     std::cout << "u: ";
     for (int i = 0; i < num_constr; i++){
          std::cout << u[i] << " , ";
     }
     std::cout << std::endl;

     // TODO: update osqp problem
}

int QP::get_num_vars(){
     return num_vars;
}

int QP::get_num_constr(){
     return num_constr;
}

c_float* QP::get_P_x(){
     return P_x;
}

c_float* QP::get_q(){
     return q;
}

c_float* QP::get_l(){
     return l;
}

c_float* QP::get_A_x(){
     return A_x;
}

c_float* QP::get_u(){
     return u;
}