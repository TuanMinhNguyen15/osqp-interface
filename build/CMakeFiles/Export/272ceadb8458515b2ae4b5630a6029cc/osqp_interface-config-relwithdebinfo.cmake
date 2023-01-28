#----------------------------------------------------------------
# Generated CMake target import file for configuration "RelWithDebInfo".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "osqp_interface::osqp_interface" for configuration "RelWithDebInfo"
set_property(TARGET osqp_interface::osqp_interface APPEND PROPERTY IMPORTED_CONFIGURATIONS RELWITHDEBINFO)
set_target_properties(osqp_interface::osqp_interface PROPERTIES
  IMPORTED_LINK_INTERFACE_LANGUAGES_RELWITHDEBINFO "CXX"
  IMPORTED_LOCATION_RELWITHDEBINFO "${_IMPORT_PREFIX}/lib/osqp_interface.lib"
  )

list(APPEND _cmake_import_check_targets osqp_interface::osqp_interface )
list(APPEND _cmake_import_check_files_for_osqp_interface::osqp_interface "${_IMPORT_PREFIX}/lib/osqp_interface.lib" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
