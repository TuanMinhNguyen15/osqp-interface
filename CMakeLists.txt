cmake_minimum_required(VERSION 3.24)
project(SequentialQuadraticProgram)

find_package(osqp REQUIRED
             PATHS "D:\\osqp\\install\\")

add_executable(test ./test.cpp)
target_link_libraries(test PRIVATE osqp::osqp)

add_library(sqp STATIC ./src/util.cpp
                       ./src/qp.cpp)
target_link_libraries(sqp PUBLIC osqp::osqp)
target_include_directories(sqp PUBLIC 
                               $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>  
                               $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}/sqp>)


add_executable(csc_gen_demo ./examples/csc_gen_demo.cpp)
target_link_libraries(csc_gen_demo PRIVATE sqp)

add_executable(qp_demo ./examples/qp_demo.cpp)
target_link_libraries(qp_demo PRIVATE sqp)

# add_executable(row_demo ./examples/row_demo.cpp)
# target_link_libraries(row_demo PRIVATE sqp)

# add_executable(param_demo ./examples/param_demo.cpp)
# target_link_libraries(param_demo PRIVATE sqp)