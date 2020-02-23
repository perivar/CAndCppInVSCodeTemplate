cmake_minimum_required(VERSION 3.0.0)
project(vst3test VERSION 0.1.0)

#  include CTest
include(CTest)
enable_testing()

# Bring the headers, such as ftypes.h into the project
include_directories(include)
 
# Can manually add the sources using the set command as follows:
# set(sources src/mainapp.cpp src/student.cpp)
set(sources vst3test.cpp)
 
# However, the file(GLOB...) allows for wildcard additions:
# file(GLOB sources "*.cpp" "*.h")
# file(GLOB_RECURSE sources "*.cpp")

set(target vst3test)
add_executable(${target} ${sources})

# if cmake complains about not knowing what linker language to use
# set_target_properties(${target} PROPERTIES LINKER_LANGUAGE CXX)

# if cmake complains
target_compile_options( ${target} PUBLIC -std=c++17 -fpermissive -w -Wall )

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
