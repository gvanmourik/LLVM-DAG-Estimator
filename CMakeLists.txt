cmake_minimum_required (VERSION 3.9)

# Silence an error about the version option to project
# this will need to be changed if we ever bump the version to 3.0 and put the
# version stuff in the project command itself
#if (NOT CMAKE_VERSION VERSION_LESS 3.0)
#  cmake_policy(SET CMP0048 OLD)
#endif()
#cmake_policy(SET CMP0025 NEW)
#cmake_policy(SET CMP0028 OLD)

project (LLVMEstimator)
# The version number.
set (LLVMEstimator_VERSION_MAJOR 0)
set (LLVMEstimator_VERSION_MINOR 1)

add_subdirectory(src)
add_subdirectory(bin)

# Create a directory to dump the .dot files
file(MAKE_DIRECTORY dotFiles)

# OPTION(DEFINE_GEN_DOT 
# 	   "Generate a dot file graph for each llvm function analyzed. 
# 	    {Stored at dotFiles/}" OFF) # Disabled by default

# IF(DEFINE_GEN_DOT)
# 	message(STATUS "defining GEN_DOT...")
#     add_definitions(-DGEN_DOT)
# ENDIF(DEFINE_GEN_DOT)
