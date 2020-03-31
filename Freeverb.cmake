cmake_minimum_required (VERSION 3.5)
project (freeverb)

message(STATUS  "Building on ${CMAKE_SYSTEM_NAME}")
message(STATUS  "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS  "Configuration types: ${CMAKE_CONFIGURATION_TYPES}")

# Name your plugin here
set(PLUGIN_NAME freeverb)

set(CMAKE_VERBOSE_MAKEFILE ON)

# this is the general configuration
# disabled - use flags per target instead by using target_compile_options()
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17 -Wall")
# use set_target_properties instead of CMAKE_CXX_FLAGS to set the CXX_STANDARD flag

set(PLUGIN_COMPILE_FLAGS)

# flags used for Debug, Release and Tracer builds
# set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVST_API -DWIN32 -D_WIN32_WINNT=0x0501 -DWINVER=0x0501 -D_CRT_SECURE_NO_DEPRECATE")

# TODO: use generator expressions instead $<$<CONFIG:RELEASE>:-O2->
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    message (STATUS "Building a debug build")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -D_DEBUG")
    list(APPEND PLUGIN_COMPILE_FLAGS "-g" "-D_DEBUG")    
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    message (STATUS "Building a release build")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    list(APPEND PLUGIN_COMPILE_FLAGS "-O2")    
endif()

# instead of global linker flags, set per target using target_link_options()
set(PLUGIN_LINKER_FLAGS)

# compile the runtime as static (this will make sure that LIBGCC_S_SEH-1.DLL et al. won't show up in Dependency Walker)
# set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
list(APPEND PLUGIN_LINKER_FLAGS "-static" "-static-libgcc" "-static-libstdc++")

# ensure we are using the windows subsystem
# set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--subsystem,windows")
# set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--subsystem,windows")
list(APPEND PLUGIN_LINKER_FLAGS "-Wl,--subsystem,windows")

# export no symbols by default
# instead of global flags, set per target in the set_target_properties() method
# set(CMAKE_C_VISIBILITY_PRESET hidden)
# set(CMAKE_CXX_VISIBILITY_PRESET hidden)
# set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

# set(CMAKE_RC_COMPILER_INIT windres)
# ENABLE_LANGUAGE(RC)
# set(CMAKE_RC_COMPILE_OBJECT
# "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")

# build the SDK as a static library
set(VST_SDK_2_4_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/VST2_SDK")
add_library(SDK2_4 STATIC
    ${VST_SDK_2_4_ROOT}/public.sdk/source/vst2.x/audioeffect.cpp
    ${VST_SDK_2_4_ROOT}/public.sdk/source/vst2.x/audioeffectx.cpp    
    ${VST_SDK_2_4_ROOT}/public.sdk/source/vst2.x/vstplugmain.cpp
    
    ${VST_SDK_2_4_ROOT}/pluginterfaces/vst2.x/aeffect.h
    ${VST_SDK_2_4_ROOT}/pluginterfaces/vst2.x/aeffectx.h
    ${VST_SDK_2_4_ROOT}/pluginterfaces/vst2.x/vstfxstore.h
)

# define output directory
set_target_properties(SDK2_4 PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# bring in the headers for the library
target_include_directories(SDK2_4
  PUBLIC
  ${VST_SDK_2_4_ROOT}
)

# compile options per target
target_compile_options(SDK2_4
  PUBLIC 
  ${PLUGIN_COMPILE_FLAGS}
)

# build the Components as a static library
set(FREEVERB_COMPONENT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/freeverb.components")
add_library (FREEVERB_COMPONENTS STATIC
  ${FREEVERB_COMPONENT_ROOT}/allpass.cpp
  ${FREEVERB_COMPONENT_ROOT}/allpass.hpp
  ${FREEVERB_COMPONENT_ROOT}/comb.cpp
  ${FREEVERB_COMPONENT_ROOT}/comb.hpp
  ${FREEVERB_COMPONENT_ROOT}/revmodel.cpp
  ${FREEVERB_COMPONENT_ROOT}/revmodel.hpp
  ${FREEVERB_COMPONENT_ROOT}/tuning.h
)

# define output directory
set_target_properties(FREEVERB_COMPONENTS PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# bring in the headers for the library
target_include_directories(FREEVERB_COMPONENTS
  PUBLIC
  ${FREEVERB_COMPONENT_ROOT}
)

# compile options per target
target_compile_options(FREEVERB_COMPONENTS
  PUBLIC 
  ${PLUGIN_COMPILE_FLAGS}
)

# set plugin root and plugin sources
set(PLUGIN_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/freeverb-vst2.4")
set(PLUGIN_SRCS
  ${PLUGIN_ROOT}/freeverb.cpp
  ${PLUGIN_ROOT}/freeverb.hpp
  ${PLUGIN_ROOT}/FreeverbMain.cpp
#   ${PLUGIN_ROOT}/Freeverb4.def
)

# set plugin resources
set(RESOURCE_SRCS
  
)

# for the DEF file check GENERATE_EXPORT_HEADER
# http://gernotklingler.com/blog/creating-using-shared-libraries-different-compilers-different-operating-systems/

# create a VST using MODULE or SHARED
add_library(${PLUGIN_NAME} SHARED
	  ${PLUGIN_SRCS}
	  ${RESOURCE_SRCS}
)

# define output directory
set_target_properties(${PLUGIN_NAME} PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib
  RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/VST2
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  CXX_EXTENSIONS OFF
)

# ensure we don't add the 'lib' prefix and change the suffix to .vst
set_target_properties(${PLUGIN_NAME} PROPERTIES 
  # stop CMake from prepending `lib` to library names
  PREFIX ""

  # For vst2's keep the dll suffix
  # SUFFIX ".dll"

  # export no symbols by default
  C_VISIBILITY_PRESET hidden
  CXX_VISIBILITY_PRESET hidden
  VISIBILITY_INLINES_HIDDEN 1
)

# Bring the headers, such as aeffeditor.h into the project
target_include_directories(${PLUGIN_NAME} PUBLIC ${VST_SDK_2_4_ROOT})
target_include_directories(${PLUGIN_NAME} PUBLIC ${FREEVERB_COMPONENT_ROOT})

# link to the sdk and components libraries
target_link_libraries(${PLUGIN_NAME} 
  PRIVATE 
  SDK2_4 
  FREEVERB_COMPONENTS
)

# set link flags per target
target_link_options(${PLUGIN_NAME} 
  PRIVATE 
  ${PLUGIN_LINKER_FLAGS}
)

# set compile options per target
target_compile_options(${PLUGIN_NAME} 
  PUBLIC 
  ${PLUGIN_COMPILE_FLAGS}
)