
#-------------------------------------------------------------------------------
# Checks
#-------------------------------------------------------------------------------

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR OR EXISTS "${CMAKE_BINARY_DIR}/CMakeLists.txt")
    message(SEND_ERROR "In-source builds are not allowed. Please create a separate build directory and run 'cmake /path/to/folder [options]' there.")
    message(FATAL_ERROR "You can remove the file \"CMakeCache.txt\" and directory \"CMakeFiles\" in ${CMAKE_SOURCE_DIR}.")
endif()

#-------------------------------------------------------------------------------
# Platform Detection
#-------------------------------------------------------------------------------
get_directory_property(hasParent PARENT_DIRECTORY)

if(APPLE)
    if(hasParent)
        set(SMTG_MAC TRUE PARENT_SCOPE)
    else()
        set(SMTG_MAC TRUE)
    endif()
elseif(UNIX OR ANDROID_PLATFORM)
    if(hasParent)
        set(SMTG_LINUX TRUE PARENT_SCOPE)
    else()
        set(SMTG_LINUX TRUE)
    endif()
elseif(WIN32)
    if(hasParent)
        set(SMTG_WIN TRUE PARENT_SCOPE)
    else()
        set(SMTG_WIN TRUE)
    endif()
endif()

#-------------------------------------------------------------------------------
# Global Settings
#-------------------------------------------------------------------------------

if(SMTG_WIN)
    option(SMTG_USE_STATIC_CRT "use static CRuntime on Windows (option /MT)" OFF)
endif()

set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type")

# Export no symbols by default
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN 1)

if(SMTG_LINUX)
    set(common_linker_flags "-Wl,--no-undefined")
    set(CMAKE_MODULE_LINKER_FLAGS "${common_linker_flags}" CACHE STRING "Module Library Linker Flags")
    set(CMAKE_SHARED_LINKER_FLAGS "${common_linker_flags}" CACHE STRING "Shared Library Linker Flags")
endif()

# Output directories
# XCode is creating the "Debug/Release" folder on its own and does not need to be added.
if(SMTG_WIN OR (SMTG_MAC AND CMAKE_GENERATOR STREQUAL Xcode))
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
else ()
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}")
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}")
endif()

if(SMTG_MAC)
    if(NOT DEFINED ENV{XCODE_VERSION})
        execute_process(COMMAND xcodebuild -version OUTPUT_VARIABLE XCODE_VERSION ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REGEX MATCH "Xcode [0-9\\.]+" XCODE_VERSION "${XCODE_VERSION}")
        string(REGEX REPLACE "Xcode ([0-9\\.]+)" "\\1" XCODE_VERSION "${XCODE_VERSION}")
    endif()
    message(STATUS "Building with Xcode version: ${XCODE_VERSION}")
    # macOS defaults
    if(NOT DEFINED ENV{MACOSX_DEPLOYMENT_TARGET})
        if(XCODE_VERSION VERSION_GREATER "7.9")
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.10" CACHE STRING "macOS deployment target")
        else()
            set(CMAKE_OSX_DEPLOYMENT_TARGET "10.8" CACHE STRING "macOS deployment target")
        endif()
        message(STATUS "macOS Deployment Target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
    if(NOT DEFINED ENV{SDKROOT})
        execute_process(COMMAND xcrun --sdk macosx --show-sdk-path OUTPUT_VARIABLE CMAKE_OSX_SYSROOT OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif()
endif()

# Generation Settings
#set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo")
set(CMAKE_CONFIGURATION_TYPES "Debug;Release")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON CACHE STRING "Generate compile commands" FORCE)

# Put predefined targets like "ALL_BUILD" and "ZERO_CHECK" into a separate folder within the IDE.
set_property(GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER Predefined)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

add_compile_options($<$<CONFIG:Debug>:-DDEVELOPMENT=1>)
add_compile_options($<$<CONFIG:Release>:-DRELEASE=1>)
add_compile_options($<$<CONFIG:RelWithDebInfo>:-DRELEASE=1>)

if(SMTG_WIN)
    # Changed by PIN: 25.02.2020

    # turn on all warnings
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

    # -mstackrealign Realign the stack at entry. 
    # On the Intel x86, the -mstackrealign option will generate an alternate prologue and epilogue 
    # that realigns the runtime stack if necessary. 
    # This supports mixing legacy codes that keep a 4-byte aligned stack with modern codes that keep a 
    # 16-byte stack for SSE compatibility. See also the attribute force_align_arg_pointer, applicable to individual functions.
    # set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mstackrealign")
    # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mstackrealign")
    
    # -Wl,--no-undefined linker option can be used when building shared library, undefined symbols will be shown as linker errors.
    # PIN: Compile the runtime as static so that the vst3 work independently
    # set(common_linker_flags "-static-libgcc -static-libstdc++ -Wl,--no-undefined")    
    # add subsystem windows to modules and shared libraries
    set(common_linker_flags "-Wl,--no-undefined -Wl,--subsystem,windows")
    set(CMAKE_MODULE_LINKER_FLAGS "${common_linker_flags}" CACHE STRING "Module Library Linker Flags")
    set(CMAKE_SHARED_LINKER_FLAGS "${common_linker_flags}" CACHE STRING "Shared Library Linker Flags")

    # The <experimental/filesystem> header is deprecated. It is superseded by the C++17 <filesystem> header.
    # set CXX standard to 17
    set(CMAKE_CXX_STANDARD 17) # C++17...
    set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
    set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

    # ensure the symbols GetPluginFactory are exported
    # these are set in AddSMTGLibrary.cmake and not here!
    # add_definitions(-DCMAKE_WINDOWS_EXPORT_ALL_SYMBOLS=TRUE -DBUILD_SHARED_LIBS=TRUE)
    
    # add_compile_options(
        # -g                          # Generate debugging information`for gdp into the file
        
        # # -Wpedantic                # Issue all the warnings demanded by strict ISO C and ISO C++
        # # -Wextra                   # This enables some extra warning flags that are not enabled by -Wall. (same as -W)
        # -Wall                       # Recommended compiler warnings (enable a set of warning, actually not all.)

        # -Wno-return-type            # ignore no return warnings
        # -Wno-conversion-null        # ignore null conversion warnings
        # -Wno-unused-parameter       # ignore unused parameter warnings
        # -Wno-unused-variable        # ignore unused variable warnings
        # -Wno-unused-but-set-variable# ignore unused and set variable warnings
    # )

    # ORIGINAL:
    # add_compile_options(/MP)                            # Multi-processor Compilation
    # if(NOT ${CMAKE_GENERATOR} MATCHES "ARM")
    #     add_compile_options($<$<CONFIG:Debug>:/ZI>)     # Program Database for Edit And Continue
    # endif()
    # if(SMTG_USE_STATIC_CRT)
    #     add_compile_options($<$<CONFIG:Debug>:/MTd>)    # Runtime Library: /MTd = MultiThreaded Debug Runtime
    #     add_compile_options($<$<CONFIG:Release>:/MT>)   # Runtime Library: /MT  = MultiThreaded Runtime
    # else()
    #     add_compile_options($<$<CONFIG:Debug>:/MDd>)    # Runtime Library: /MDd = MultiThreadedDLL Debug Runtime
    #     add_compile_options($<$<CONFIG:Release>:/MD>)   # Runtime Library: /MD  = MultiThreadedDLL Runtime
    # endif()
endif()

# Add colors to clang output when using Ninja
# See: https://github.com/ninja-build/ninja/wiki/FAQ
if (UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "Clang" AND CMAKE_GENERATOR STREQUAL "Ninja")
    add_compile_options(-fcolor-diagnostics)
endif()
