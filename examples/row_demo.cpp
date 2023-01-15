#include "sqp/util.hpp"
#include <iostream>

int main(){
    ROW row1,row2;

    row1.entries = {{0,10},{2,30}};
    row2.entries = {{1,20},{2,40}};

    ROW row3;
    row3 = row1 - row2;

    std::cout << "row1: " ;
    for (auto row1_entry : row1.entries){
        std::cout << "<" << row1_entry.first << " , " << row1_entry.second << "> , ";
    }
    std::cout << std::endl;

    std::cout << "row2: " ;
    for (auto row2_entry : row2.entries){
        std::cout << "<" << row2_entry.first << " , " << row2_entry.second << "> , ";
    }
    std::cout << std::endl;

    std::cout << "row3: " ;
    for (auto row3_entry : row3.entries){
        std::cout << "<" << row3_entry.first << " , " << row3_entry.second << "> , ";
    }
    std::cout << std::endl;
}