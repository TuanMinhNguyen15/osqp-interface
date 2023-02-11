cmake_minimum_required(VERSION 3.24)
project(OSQP_INTERFACE)

find_package(osqp REQUIRED
             PATHS "D:\\osqp\\install\\")


# Create static library
add_library(osqp_interface STATIC ./src/util.cpp
                                  ./src/qp.cpp)
target_link_libraries(osqp_interface PUBLIC osqp::osqp)
target_include_directories(osqp_interface PUBLIC 
                               "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"  
                               "$<INSTALL_INTERFACE:include>")

# Install static library
install(TARGETS osqp_interface
        EXPORT  ${PROJECT_NAME}
        ARCHIVE       DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY       DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME       DESTINATION "${CMAKE_INSTALL_BINDIR}")

install(DIRECTORY ./include/
        DESTINATION include)

install(EXPORT ${PROJECT_NAME}
        FILE osqp_interface-config.cmake
        NAMESPACE osqp_interface::
        DESTINATION cmake)

# Create demo executables
add_executable(demo1 ./examples/demo1.cpp)
target_link_libraries(demo1 PRIVATE osqp_interface)

add_executable(demo2 ./examples/demo2.cpp)
target_link_libraries(demo2 PRIVATE osqp_interface)
