#pragma once

#include "osqp.h"
#include "osqp-interface/util.hpp"
#include <iostream>
#include <set>
#include <utility>
#include <string>
#include <limits>

enum class TYPE {LOWER_BOUND, 
                 UPPER_BOUND, 
                 CONSTRAINT, 
                 QUADRATIC_COST, 
                 LINEAR_COST, 
                 UNDEFINED};

struct PARAM_DUMMY{
    void* param_ptr;
    int index;
    int var_index;
    int var_index2 = -1;
    c_float scale = 1;

    PARAM_DUMMY operator -();
};

PARAM_DUMMY operator * (c_float coef, PARAM_DUMMY param_dummy);
ROW operator * (PARAM_DUMMY param_dummy, ROW row);

struct Expression{
    ROW     linear_terms;
    ROW_COL quadratic_terms;
    std::vector<PARAM_DUMMY> param_dummies;
    Expression();
    Expression operator - ();
};

Expression operator + (Expression exp1, Expression exp2);
Expression operator - (Expression exp1, Expression exp2);
Expression operator * (float coef, Expression exp);
Expression operator * (Expression exp1, Expression exp2); // quadraic term!!!
Expression operator * (PARAM_DUMMY param_dummy, Expression exp);


class QP {
    public:
        class Variable{
            public:
                Variable(int size);
                std::vector<c_float> solution;
                Expression operator [] (int index);

            private:
                int col_start = -1;
                int size;
                friend class QP;
        };
        
        class Parameter{
            public:
                Parameter(int size);
                void set_data(std::vector<c_float> data_new);
                PARAM_DUMMY operator [] (int index);

            private:
                int size;
                QP *qp;
                char type; 
                std::set<char> avail_types = {'P','q','l','A','u'};
                
                std::vector<c_float> data;
                std::vector<std::vector<int>> var_indices,var_indices2,constr_indices;  
                std::vector<std::vector<c_float>> scales;
                std::vector<std::vector<TYPE>> types;
                std::vector< std::vector< std::pair<c_float*,int>>> targets;

                friend class QP;
        };

        struct QP_Params{
            std::vector<Variable*> vars;
            std::vector<Parameter*> params;
        };

        QP(QP_Params qp_params);
        ~QP();

        void add_constraint(c_float     lb, Expression exp, c_float     ub);
        void add_constraint(PARAM_DUMMY lb, Expression exp, c_float     ub);
        void add_constraint(c_float     lb, Expression exp, PARAM_DUMMY ub);
        void add_constraint(PARAM_DUMMY lb, Expression exp, PARAM_DUMMY ub);
        void add_cost(Expression exp);
        void formulate();
        void update();
        void solve();
        void print_problem();

        int get_num_vars();
        int get_num_constr();
        c_float get_obj_val();
        std::string get_status();
        OSQPSettings * get_settings();

        // limits
        static const c_float POS_INF;
        static const c_float NEG_INF;


    
    private:
        QP_Params qp_params_;
        c_int num_vars = 0;
        c_int num_constr = 0;

        Expression cost;
        CSC_GEN P_matrix;
        CSC_GEN A_matrix;

        std::vector<c_float> l_vec,u_vec,q_vec;
        std::vector<c_float> l_original,u_original,q_original;

        // OSQP variables
        OSQPWorkspace *work;
        OSQPSettings  *settings;
        OSQPData      *data;

        // P matrix
        c_float *P_x;
        c_int    P_nnz;
        c_int   *P_i;
        c_int   *P_p;

        // q vector
        c_float *q;

        // A matrix
        c_float *A_x;
        c_int    A_nnz;
        c_int   *A_i;
        c_int   *A_p;

        // l & u vectors
        c_float *l,*u;

};
