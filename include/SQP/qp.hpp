#pragma once

#include "sqp/util.hpp"
#include <iostream>
#include <set>

struct PARAM_DUMMY{
    c_float param_data;
    void* param_ptr;
    int index;
};

ROW operator * (PARAM_DUMMY param_dummy, ROW &row);


class QP {
    public:
        struct Variable{
            // public
            Variable(int size);
            std::vector<c_float> sol;
            ROW operator [] (int index);

            // private
            int col_start = -1;
            int size;
        };
        
        struct Parameter{
            // public
            Parameter(int size, char type);
            void update(std::vector<c_float> data_new);
            PARAM_DUMMY operator [] (int index);

            // private
            int size;
            QP *qp;
            char type; 
            std::set<char> avail_types = {'P','q','l','A','u'};
            
            std::vector<c_float> data;
            std::vector<std::vector<int>> var_indices,constr_indices;  // Book Kepping
            std::vector<std::vector<int>> target_indices;  // Important!
            c_float *target;
        };

        struct QP_Params{
            std::vector<Variable*> vars;
            std::vector<Parameter*> params;
        };

        QP(QP_Params qp_params);

        void add_constraint(c_float     lb, ROW row, c_float     ub);
        void add_constraint(PARAM_DUMMY lb, ROW row, c_float     ub);
        void add_constraint(c_float     lb, ROW row, PARAM_DUMMY ub);
        void add_constraint(PARAM_DUMMY lb, ROW row, PARAM_DUMMY ub);
        void formulate();
        void update();

        int get_num_vars();
        int get_num_constr();
        c_float* get_P_x();
        c_float* get_q();
        c_float* get_l();
        c_float* get_A_x();
        c_float* get_u();

    
    private:
        QP_Params qp_params_;
        int num_vars = 0;
        int num_constr = 0;

        CSC_GEN P_matrix;
        CSC_GEN A_matrix;

        std::vector<c_float> l_vec,u_vec;

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

        // n = number of variables
        c_int n;

        // m = number of constraints
        c_int m;

};
