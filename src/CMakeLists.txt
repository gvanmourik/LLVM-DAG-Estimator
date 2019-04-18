
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

# Set your project compile flags.
# E.g. if using the C++ header files
# you will need to enable C++11 support
# for your compiler.

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Find the libraries that correspond to the LLVM components
# that we wish to use
llvm_map_components_to_libnames(llvm_libs
  support
  core
  irreader
  x86asmparser
  x86asmprinter
  x86codegen
  x86desc
  x86disassembler
  x86info
  x86utils
  mcjit
  passes
)
execute_process(COMMAND ${LLVM_DIR}/../../../bin/llvm-config --cxxflags 
                OUTPUT_VARIABLE proc_output)
string(REPLACE " " ";" inherited_cxx_flags ${proc_output})
list(REMOVE_ITEM inherited_cxx_flags "-fno-exceptions")
list(REMOVE_ITEM inherited_cxx_flags "-fno-rtti")

set(SOURCES
  DAGBuilder.cpp
  DAGNode.cpp
  FunctionInfoPass.cpp
  ModuleInfoPass.cpp
  execute.cpp
  optimize.cpp
  parseIr.cpp
  runAnalysis.cpp
  test1.cpp
  test2.cpp
  test3.cpp
)

add_library(llvmEstimator ${SOURCES})
add_library(llvmEstimator::llvmEstimator ALIAS llvmEstimator)


target_link_libraries(llvmEstimator ${llvm_libs})
target_compile_options(llvmEstimator PUBLIC ${inherited_cxx_flags})

target_include_directories(llvmEstimator PUBLIC
  $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
  $<INSTALL_INTERFACE:include>
)

install(TARGETS llvmEstimator EXPORT llvmEstimator
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        RUNTIME DESTINATION bin
        INCLUDES DESTINATION include)

install(EXPORT llvmEstimator
        FILE llvmEstimatorTargets.cmake
        NAMESPACE llvmEstimator::
        DESTINATION cmake)


include(CMakePackageConfigHelpers)
configure_package_config_file(cmake/llvmEstimatorConfig.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/llvmEstimatorConfig.cmake
        INSTALL_DESTINATION cmake)
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/llvmEstimatorConfigVersion.cmake
        VERSION "${LLVMEstimator_VERSION_MAJOR}.${LLVMEstimator_VERSION_MINOR}"
        COMPATIBILITY SameMajorVersion)

install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/llvmEstimatorConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/llvmEstimatorConfigVersion.cmake
  DESTINATION cmake
)

