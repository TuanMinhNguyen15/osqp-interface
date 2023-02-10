#include "osqp-interface/qp.hpp"

// ---------- Variable ----------
QP::Variable::Variable():size(0){}

QP::Variable::Variable(int size):size(size){
     solution.resize(size);
}

void QP::Variable::update_size(int size){
     Variable var(size);
     *this = var;
}

Expression QP::Variable::operator[] (int index){
     Expression exp;
     exp.linear_terms.entries.clear();
     exp.linear_terms.entries.insert({col_start+index,1});

     return exp;
}

// ---------- Parameter ----------
QP::Parameter::Parameter():size(0){}

QP::Parameter::Parameter(int size):size(size){
     data.resize(size,0);
     types.resize(size);
     var_indices.resize(size);
     var_indices2.resize(size);
     constr_indices.resize(size);
     scales.resize(size);
     targets.resize(size);
}

void QP::Parameter::update_size(int size){
     Parameter param(size);
     *this = param;
}

void QP::Parameter::set_data(std::vector<c_float> data_new){
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

PARAM_DUMMY PARAM_DUMMY::operator-(){
     PARAM_DUMMY param_dummy;
     param_dummy.param_ptr = param_ptr;
     param_dummy.index = index;
     param_dummy.var_index = var_index;
     param_dummy.scale *= -1;

     return param_dummy;
}

PARAM_DUMMY operator * (c_float coef, PARAM_DUMMY param_dummy){
     param_dummy.scale = coef;
     
     return param_dummy;
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
     exp_out.quadratic_terms = exp1.quadratic_terms + exp2.quadratic_terms;

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
     exp_out.quadratic_terms = exp1.quadratic_terms - exp2.quadratic_terms;

     // Scaling
     for (auto &param_dummy : exp2.param_dummies){
          param_dummy.scale *= -1;
     }

     // Params
     exp_out.param_dummies.insert(exp_out.param_dummies.end(),exp1.param_dummies.begin(),exp1.param_dummies.end());
     exp_out.param_dummies.insert(exp_out.param_dummies.end(),exp2.param_dummies.begin(),exp2.param_dummies.end());

     return exp_out;
}

Expression Expression::operator-(){
     Expression exp_out;
     exp_out.linear_terms = linear_terms;
     exp_out.quadratic_terms = quadratic_terms;
     exp_out.param_dummies = param_dummies;

     // Linear terms
     for (auto &linear_term : exp_out.linear_terms.entries){
          linear_term.second *= -1;
     }

     // Quadratic terms
     for (auto &quadratic_term : exp_out.quadratic_terms.entries){
          quadratic_term.second *= -1;
     }

     return exp_out;
}

Expression operator * (float coef, Expression exp){
     // Linear terms
     exp.linear_terms = coef*exp.linear_terms;

     // Quadratic terms
     exp.quadratic_terms = coef*exp.quadratic_terms;

     return exp;
}

Expression operator * (PARAM_DUMMY param_dummy, Expression exp){

     auto entry_ptr = exp.linear_terms.entries.begin();  
     auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);

     entry_ptr->second = 0; // param_dummy.param_data

     param_dummy.var_index = entry_ptr->first;
   
     exp.param_dummies.push_back(param_dummy);
     return exp;
}

Expression operator * (Expression exp1, Expression exp2){
     Expression exp_out;

     // Params
     exp_out.param_dummies.insert(exp_out.param_dummies.end(),exp1.param_dummies.begin(),exp1.param_dummies.end());
     exp_out.param_dummies.insert(exp_out.param_dummies.end(),exp2.param_dummies.begin(),exp2.param_dummies.end());
     auto param_dummy_ptr = exp_out.param_dummies.begin();

     // Convert linear terms to quadratic terms
     for (auto linear_term_exp1 : exp1.linear_terms.entries){
          for (auto linear_term_exp2 : exp2.linear_terms.entries){
               if (linear_term_exp1.first >= linear_term_exp2.first){
                    exp_out.quadratic_terms.entries.insert({{linear_term_exp1.first,linear_term_exp2.first},
                                                             linear_term_exp1.second*linear_term_exp2.second*2});
                    
                    // Parameters
                    if (param_dummy_ptr != exp_out.param_dummies.end()){
                         param_dummy_ptr->var_index  = linear_term_exp1.first;
                         param_dummy_ptr->var_index2 = linear_term_exp2.first;
                         param_dummy_ptr->scale = (linear_term_exp1.first == linear_term_exp2.first) ? param_dummy_ptr->scale*2 : param_dummy_ptr->scale*0.5*2;
                    }      
               }
               else{
                    exp_out.quadratic_terms.entries.insert({{linear_term_exp2.first,linear_term_exp1.first},
                                                             linear_term_exp1.second*linear_term_exp2.second*2});

                    // Parameters
                    if (param_dummy_ptr != exp_out.param_dummies.end()){
                         param_dummy_ptr->var_index  = linear_term_exp2.first;
                         param_dummy_ptr->var_index2 = linear_term_exp1.first;
                         param_dummy_ptr->scale = (linear_term_exp1.first == linear_term_exp2.first) ? param_dummy_ptr->scale*2 : param_dummy_ptr->scale*0.5*2;

                    }
               }

          }
     }

     return exp_out;
}


// ---------- QP ----------
QP::QP(){}

QP::QP(QP_Params qp_params):qp_params_(qp_params){
     bool is_zero_size = false;
     bool is_no_variable = true;
     // variables
     for (auto var : qp_params_.vars){
          is_no_variable = false;
          if (var->size == 0){
               is_zero_size = true;
               break;
          }

          if (num_vars == 0){
               var->col_start = 0;  
          }
          else{
               var->col_start = num_vars;
          }
          num_vars += var->size;
     }

     // parameters
     for (auto param : qp_params_.params){
          if (param->size == 0){
               is_zero_size = true;
               break;
          }

          param->qp = this;
     }

     if (is_no_variable || is_zero_size){
          std::cout << "Error: Either QP problem has no variable or at least one variable/parameter has zero size\n";
     }
     else{
          settings = (OSQPSettings *)c_malloc(sizeof(OSQPSettings));
          data     = (OSQPData *)c_malloc(sizeof(OSQPData));
          osqp_set_default_settings(settings);
          settings->alpha = 1.0; // Change alpha parameter

          A_matrix.update_num_cols(num_vars);
          P_matrix.update_num_cols(num_vars);
          cost.linear_terms.entries.clear();
          cost.quadratic_terms.entries.clear();
          cost.param_dummies.clear();

          is_setup = true;
     }    
}

void QP::update_qp_params(QP_Params qp_params){
     QP qp(qp_params);
     *this = qp;
}

QP::~QP(){
     if (is_formulated){
          std::cout << "I am done\n";
          osqp_cleanup(work);
          if (data) {
               if (data->A) c_free(data->A);
               if (data->P) c_free(data->P);
               c_free(data);
          }
          if (settings) c_free(settings);

          std::cout << "?????!!!!!!????????\n";
     }
}

void QP::add_constraint(c_float lb, Expression exp, c_float ub){
     if (!is_setup){
          std::cout << "Error: QP problem needs to be setup first before adding constraints\n";
          return;
     }

     l_vec.push_back(lb);
     A_matrix.add_row(exp.linear_terms);
     u_vec.push_back(ub);

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          param_ptr->types[param_dummy.index].push_back(TYPE::CONSTRAINT);
          param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index);
          param_ptr->var_indices2[param_dummy.index].push_back(-1); 
          param_ptr->constr_indices[param_dummy.index].push_back(num_constr);
          param_ptr->scales[param_dummy.index].push_back(param_dummy.scale);
     }

     num_constr++;
}

void QP::add_constraint(PARAM_DUMMY lb, Expression exp, c_float ub){
     if (!is_setup){
          std::cout << "Error: QP problem needs to be setup first before adding constraints\n";
          return;
     }

     l_vec.push_back(0); // lb.param_data
     A_matrix.add_row(exp.linear_terms);
     u_vec.push_back(ub);

     QP::Parameter *param_ptr = static_cast<QP::Parameter*>(lb.param_ptr);
     param_ptr->constr_indices[lb.index].push_back(num_constr);
     param_ptr->var_indices[lb.index].push_back(-1);
     param_ptr->var_indices2[lb.index].push_back(-1);
     param_ptr->types[lb.index].push_back(TYPE::LOWER_BOUND);
     param_ptr->scales[lb.index].push_back(lb.scale);

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          param_ptr->types[param_dummy.index].push_back(TYPE::CONSTRAINT);
          param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index); 
          param_ptr->var_indices2[param_dummy.index].push_back(-1);
          param_ptr->constr_indices[param_dummy.index].push_back(num_constr);
          param_ptr->scales[param_dummy.index].push_back(param_dummy.scale);
     }

     num_constr++;
}

void QP::add_constraint(c_float lb, Expression exp, PARAM_DUMMY ub){
     if (!is_setup){
          std::cout << "Error: QP problem needs to be setup first before adding constraints\n";
          return;
     }

     l_vec.push_back(lb);
     A_matrix.add_row(exp.linear_terms);
     u_vec.push_back(0);  // ub.param_data
   

     QP::Parameter *param_ptr = static_cast<QP::Parameter*>(ub.param_ptr);
     param_ptr->constr_indices[ub.index].push_back(num_constr);
     param_ptr->var_indices[ub.index].push_back(-1);
     param_ptr->var_indices2[ub.index].push_back(-1);
     param_ptr->types[ub.index].push_back(TYPE::UPPER_BOUND);
     param_ptr->scales[ub.index].push_back(ub.scale);

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          param_ptr->types[param_dummy.index].push_back(TYPE::CONSTRAINT);
          param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index);
          param_ptr->var_indices2[param_dummy.index].push_back(-1); 
          param_ptr->constr_indices[param_dummy.index].push_back(num_constr);
          param_ptr->scales[param_dummy.index].push_back(param_dummy.scale);
     }

     num_constr++;
}

void QP::add_constraint(PARAM_DUMMY lb, Expression exp, PARAM_DUMMY ub){
     if (!is_setup){
          std::cout << "Error: QP problem needs to be setup first before adding constraints\n";
          return;
     }

     l_vec.push_back(0); // lb.param_data
     A_matrix.add_row(exp.linear_terms);
     u_vec.push_back(0); // ub.param_data

     QP::Parameter *lb_param_ptr = static_cast<QP::Parameter*>(lb.param_ptr);
     lb_param_ptr->constr_indices[lb.index].push_back(num_constr);
     lb_param_ptr->var_indices[lb.index].push_back(-1);
     lb_param_ptr->var_indices2[lb.index].push_back(-1);
     lb_param_ptr->types[lb.index].push_back(TYPE::LOWER_BOUND);
     lb_param_ptr->scales[lb.index].push_back(lb.scale);

     QP::Parameter *ub_param_ptr = static_cast<QP::Parameter*>(ub.param_ptr);
     ub_param_ptr->constr_indices[ub.index].push_back(num_constr);
     ub_param_ptr->var_indices[ub.index].push_back(-1);
     ub_param_ptr->var_indices2[ub.index].push_back(-1);
     ub_param_ptr->types[ub.index].push_back(TYPE::UPPER_BOUND);
     ub_param_ptr->scales[ub.index].push_back(ub.scale);

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          param_ptr->types[param_dummy.index].push_back(TYPE::CONSTRAINT);
          param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index);
          param_ptr->var_indices2[param_dummy.index].push_back(-1); 
          param_ptr->constr_indices[param_dummy.index].push_back(num_constr);
          param_ptr->scales[param_dummy.index].push_back(param_dummy.scale);
     }

     num_constr++;
}

void QP::add_cost(Expression exp){
     if (!is_setup){
          std::cout << "Error: QP problem needs to be setup first before adding costs\n";
          return;
     }

     cost = cost + exp;

     for (auto param_dummy : exp.param_dummies){
          auto param_ptr = static_cast<QP::Parameter*>(param_dummy.param_ptr);
          if (param_dummy.var_index2 == -1){
               // Linear term
               param_ptr->types[param_dummy.index].push_back(TYPE::LINEAR_COST);
               param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index);
               param_ptr->var_indices2[param_dummy.index].push_back(-1);
               param_ptr->constr_indices[param_dummy.index].push_back(-1);
               param_ptr->scales[param_dummy.index].push_back(param_dummy.scale);

          }
          else{
               // Quadratic term
               param_ptr->types[param_dummy.index].push_back(TYPE::QUADRATIC_COST);
               param_ptr->var_indices[param_dummy.index].push_back(param_dummy.var_index);
               param_ptr->var_indices2[param_dummy.index].push_back(param_dummy.var_index2);
               param_ptr->constr_indices[param_dummy.index].push_back(-1);
               param_ptr->scales[param_dummy.index].push_back(param_dummy.scale);
          }
     }
}

void QP::formulate(){
     // check for setup
     if (!is_setup){
          std::cout << "Error: QP problem needs to be setup first before being formulated\n";
          return;
     }

     // Objective Cost
          // Quadratic Cost
     if (cost.quadratic_terms.entries.size() == 0){
          auto var = *qp_params_.vars[0];
          cost  = cost + 0*var[0]*var[0];
     }
     P_matrix.add_row_col(cost.quadratic_terms);
     P_matrix.csc_generate(P_x,P_nnz,P_i,P_p);

          // Linear Cost
     for (int i = 0; i < num_vars; i++){
          auto found = cost.linear_terms.entries.find(i);
          if (found != cost.linear_terms.entries.end()){
               // Found
               q_vec.push_back(cost.linear_terms.entries[i]);
          }
          else{
               // Not Found
               q_vec.push_back(0);
          }
     }
     q = &q_vec[0];
     q_original = q_vec;

     // Constraints
     if (num_constr == 0){
          auto var = *qp_params_.vars[0];
          add_constraint(NEG_INF,0*var[0],POS_INF);
     }
     A_matrix.csc_generate(A_x,A_nnz,A_i,A_p);
     l = &l_vec[0];
     u = &u_vec[0];
     l_original = l_vec;
     u_original = u_vec;

     // Parameters
     for (auto param : qp_params_.params){
          for (int i = 0; i < param->size; i++){
               param->targets[i].resize(param->types[i].size());
               for (int j = 0; j < param->types[i].size(); j++){
                    switch (param->types[i][j]){
                         case TYPE::LOWER_BOUND:
                              param->targets[i][j].first  = l;
                              param->targets[i][j].second = param->constr_indices[i][j];
                              break;

                         case TYPE::UPPER_BOUND:
                              param->targets[i][j].first  = u;
                              param->targets[i][j].second = param->constr_indices[i][j];
                              break;

                         case TYPE::CONSTRAINT:
                              param->targets[i][j].first  = A_x;
                              param->targets[i][j].second = A_p[param->var_indices[i][j]] + param->constr_indices[i][j];
                              break;

                         case TYPE::LINEAR_COST:
                              param->targets[i][j].first = q;
                              param->targets[i][j].second = param->var_indices[i][j];
                              break;

                         case TYPE::QUADRATIC_COST:
                              param->targets[i][j].first = P_x;
                              param->targets[i][j].second = P_p[param->var_indices[i][j]] + param->var_indices2[i][j];
                              break;

                         default:
                              std::cout << "oh no...\n";
                              break;
                    }
               }
          }

     }

     // OSQP Setup
     data->n = num_vars;
     data->m = num_constr;
     data->P = csc_matrix(data->n, data->n, P_nnz, P_x, P_i, P_p);
     data->q = q;
     data->A = csc_matrix(data->m, data->n, A_nnz, A_x, A_i, A_p);
     data->l = l;
     data->u = u;

     osqp_setup(&work, data, settings);
     is_formulated = true;
     update();
}

void QP::update(){
     if (!is_formulated){
          std::cout << "Error: QP problem needs to be formulated first before being updated\n";
          return;
     }

     P_matrix.restore();
     q_vec = q_original;
     A_matrix.restore();
     l_vec = l_original;
     u_vec = u_original;
     l = &l_vec[0];
     u = &u_vec[0];
     q = &q_vec[0];

     for (auto param : qp_params_.params){
          for (int data_index = 0; data_index < param->data.size(); data_index++){
               for (auto target_index = 0; target_index < param->targets[data_index].size(); target_index++){
                    param->targets[data_index][target_index].first[param->targets[data_index][target_index].second] += param->data[data_index]*param->scales[data_index][target_index];
               }
          }
     }

     osqp_update_P(work, P_x, OSQP_NULL, P_matrix.get_nnz());
     osqp_update_A(work, A_x, OSQP_NULL, A_matrix.get_nnz());
     osqp_update_lin_cost(work,q);
     osqp_update_bounds(work,l,u);
     
}

void QP::solve(){
     if (!is_formulated){
          std::cout << "Error: QP problem needs to be formulated first before being solved\n";
          return;
     }

     osqp_solve(work);
     for (auto var : qp_params_.vars){
          for (auto i = var->col_start ; i < var->col_start + var->size ; i++){
               var->solution[i-var->col_start] = work->solution->x[i];
          }
     }

   
}

void QP::print_problem(){
     std::cout << "----- Objective Cost -----\n";
     P_matrix.print_matrix();
     
     std::cout << "q: ";
     for (int i = 0; i < num_vars; i++){
          std::cout << q[i] << " , ";
     }
     std::cout << std::endl;

     std::cout << "----- Constraints -----\n";
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

int QP::get_num_vars(){
     return num_vars;
}

int QP::get_num_constr(){
     return num_constr;
}

c_float QP::get_obj_val(){
     if (!is_setup){
          std::cout << "Error: QP problem is NOT formulated\n";
          return -1;
     }

     return work->info->obj_val;
}

std::string QP::get_status(){
     if (!is_setup){
          return "Error: QP problem is NOT formulated";
     }

     return std::string (work->info->status);
}

OSQPSettings * QP::get_settings(){
     return settings;
}


const c_float QP::POS_INF = (std::numeric_limits<c_float>::max)();
const c_float QP::NEG_INF =  std::numeric_limits<c_float>::lowest();