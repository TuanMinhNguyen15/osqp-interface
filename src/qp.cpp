#include "sqp/qp.hpp"

// ---------- Variable ----------

QP::Variable::Variable(int size):size(size){
     sol.resize(size);
}

ROW QP::Variable::operator[] (int index){
     ROW row;
     row.entries.clear();
     row.entries.insert({col_start+index,1});

     return row;
}

// ---------- Parameter ----------

QP::Parameter::Parameter(int size, char type){
     this->size = size;
     data.resize(size,0);
     target_indices.resize(size,-1);
     var_indices.resize(size,-1);
     constr_indices.resize(size,-1);

     auto found = avail_types.find(type);
     if (found != avail_types.end()){
          // Found
          this->type = type;
     }
     else{
          // Not Found
          std::cerr << "Invalid Type!\n";
     }
}

void QP::Parameter::update(std::vector<c_float> data_new){
     if (data_new.size() == size){
          data = data_new;

          // TODO: update target
          for (auto i = 0; i < size; i++){
               target[target_indices[i]] = data[i];
          }
     }
     else{
          std::cerr << "Invalid size of new data\n";
     }
}

PARAM_DUMMY QP::Parameter::operator[] (int index){
     PARAM_DUMMY param_dummy;
     param_dummy.param_data = data[index];
     param_dummy.param_ptr = this;
     param_dummy.index = index;

     return param_dummy;
  
}

ROW operator * (PARAM_DUMMY param_dummy, ROW &row){
     // TODO: * operator only works on type 'A' parameters
     if (row.entries.size() == 1){
          auto entry_ptr = row.entries.begin(); 
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);

          entry_ptr->second = param_dummy.param_data;
          param_ptr->var_indices[param_dummy.index] = entry_ptr->first;
          param_ptr->constr_indices[param_dummy.index] = param_ptr->qp->get_num_constr();
          
     }
     else{
          std::cerr << "row must have only one element!\n";
     }

     return row;
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

void QP::add_constraint(c_float lb, ROW row, c_float ub){
     l_vec.push_back(lb);
     A_matrix.add_row(row);
     u_vec.push_back(ub);

     num_constr++;
}

void QP::add_constraint(PARAM_DUMMY lb, ROW row, c_float ub){
     l_vec.push_back(lb.param_data);
     A_matrix.add_row(row);
     u_vec.push_back(ub);

     QP::Parameter *param_ptr = static_cast<QP::Parameter*>(lb.param_ptr);
     param_ptr->constr_indices[lb.index] = num_constr;

     num_constr++;
}

void QP::add_constraint(c_float lb, ROW row, PARAM_DUMMY ub){
     l_vec.push_back(lb);
     A_matrix.add_row(row);
     u_vec.push_back(ub.param_data);

     QP::Parameter *param_ptr = static_cast<QP::Parameter*>(ub.param_ptr);
     param_ptr->constr_indices[ub.index] = num_constr;

     num_constr++;
}

void QP::add_constraint(PARAM_DUMMY lb, ROW row, PARAM_DUMMY ub){
     l_vec.push_back(lb.param_data);
     A_matrix.add_row(row);
     u_vec.push_back(ub.param_data);

     QP::Parameter *lb_param_ptr = static_cast<QP::Parameter*>(lb.param_ptr);
     lb_param_ptr->constr_indices[lb.index] = num_constr;

     QP::Parameter *ub_param_ptr = static_cast<QP::Parameter*>(ub.param_ptr);
     ub_param_ptr->constr_indices[ub.index] = num_constr;

     num_constr++;
}

void QP::formulate(){
     // TODO: make sure all parameters are in used!

     A_matrix.csc_generate(A_x,A_nnz,A_i,A_p);
     l = &l_vec[0];
     u = &u_vec[0];

     for (auto param : qp_params_.params){

          switch (param->type)
          {
          case 'P':
               param->target = P_x;
               break;

          case 'q':
               param->target = q;
               break;

          case 'l':
               param->target = l;
               for (auto i = 0; i < param->size; i++){
                    param->target_indices[i] = param->constr_indices[i];
               }
               break;

          case 'A':
               param->target = A_x;
               for (auto i = 0; i < param->size; i++){
                    param->target_indices[i] = A_p[param->var_indices[i]] + param->constr_indices[i];
               }
               break;

          case 'u':
               param->target = u;
               for (auto i = 0; i < param->size; i++){
                    param->target_indices[i] = param->constr_indices[i];
               }
               break;
          
          default:
               break;
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
}

void QP::update(){
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