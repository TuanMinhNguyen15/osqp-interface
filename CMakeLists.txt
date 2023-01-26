cmake_minimum_required(VERSION 3.24)
project(SequentialQuadraticProgram)

find_package(osqp REQUIRED
             PATHS "D:\\osqp\\install\\")

add_executable(test ./test.cpp)
target_link_libraries(test PRIVATE osqp::osqp)

add_library(osqp_interface STATIC ./src/util.cpp
                       ./src/qp.cpp)
target_link_libraries(osqp_interface PUBLIC osqp::osqp)
target_include_directories(osqp_interface PUBLIC 
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
                               $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}/sqp>)



add_executable(qp_demo ./examples/qp_demo.cpp)
target_link_libraries(qp_demo PRIVATE osqp_interface)