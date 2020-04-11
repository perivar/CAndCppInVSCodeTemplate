
macro(setupPlatformToolset)
    # deprecated
    if(SMTG_RENAME_ASSERT)
        add_compile_options(-DSMTG_RENAME_ASSERT=1)
    endif()

    #------------
    if(SMTG_LINUX)
        option(SMTG_ADD_ADDRESS_SANITIZER_CONFIG "Add AddressSanitizer Config (Linux only)" OFF)
        if(SMTG_ADD_ADDRESS_SANITIZER_CONFIG)
            set(CMAKE_CONFIGURATION_TYPES "${CMAKE_CONFIGURATION_TYPES};ASan")
            add_compile_options($<$<CONFIG:ASan>:-DDEVELOPMENT=1>)
            add_compile_options($<$<CONFIG:ASan>:-fsanitize=address>)
            add_compile_options($<$<CONFIG:ASan>:-DVSTGUI_LIVE_EDITING=1>)
            add_compile_options($<$<CONFIG:ASan>:-g>)
            add_compile_options($<$<CONFIG:ASan>:-O0>)
            set(ASAN_LIBRARY asan)
            link_libraries($<$<CONFIG:ASan>:${ASAN_LIBRARY}>)
        endif()
    endif()

    #------------
    if(UNIX)
        if(XCODE)
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++14")
            set(CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++")
        elseif(SMTG_MAC)
            set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
            set(CMAKE_CXX_STANDARD 14)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
            link_libraries(c++)
        else()
            set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
            set(CMAKE_CXX_STANDARD 14)
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")
            if(ANDROID)
                set(CMAKE_ANDROID_STL_TYPE c++_static)
                link_libraries(dl)
             else()
                link_libraries(stdc++fs pthread dl)
            endif()
        endif()
    #------------
    elseif(SMTG_WIN)
        add_definitions(-DUNICODE -D_UNICODE) # Build as Windows unicode application.

        # Changed by PIN: 25.02.2020
        # The <experimental/filesystem> header is deprecated. It is superseded by the C++17 <filesystem> header.
        # set CXX standard to 17
        set(CMAKE_CXX_STANDARD 17) # C++17...
        set(CMAKE_CXX_STANDARD_REQUIRED ON) #...is required...
        set(CMAKE_CXX_EXTENSIONS OFF) #...without compiler extensions like gnu++11

        # Changed by PIN: 09.04.2020
        if(MINGW)
            # Since the Global.cmake file is loaded before any of the platform variables is set,
            # it's imppossble to know whether we are compiling on windows using MSVC or MINGW.
            # Therefore the setup info is moved here to the setupPlatformToolset macro
                
            # -Wl,--no-undefined linker option can be used when building shared library, undefined symbols will be shown as linker errors.
            set(common_linker_flags "-Wl,--no-undefined")
            set(CMAKE_MODULE_LINKER_FLAGS "${common_linker_flags}" CACHE STRING "Module Library Linker Flags")
            set(CMAKE_SHARED_LINKER_FLAGS "${common_linker_flags}" CACHE STRING "Shared Library Linker Flags")       

            # turn on all warnings
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

            # remove multichar warnings
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")

            #  -municode
            # This option is available for MinGW-w64 targets.  It causes the
            # "UNICODE" preprocessor macro to be predefined, and chooses
            # Unicode-capable runtime startup code.
            # with the -municode flag:
            # .... in function `wmain': crt0_w.c:23: undefined reference to `wWinMain'
            # without this flag:
            # .... in function `main': crt0_c.c:18: undefined reference to `WinMain'
            # disabled since we rather would want to set this on each target using 
            # set_target_properties(${target} PROPERTIES LINK_FLAGS -municode)
            # or for CMake >=3.13
            # target_link_options(${target} PRIVATE -municode)
            # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -municode")

            # Note!  -mwindows should probably not be passed directly; instead, use CMake's built-in WIN32 argument in add_executable
            # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mwindows")
            # set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mconsole")

        else()     
            add_compile_options(/MP)                            # Multi-processor Compilation
            if(NOT ${CMAKE_GENERATOR} MATCHES "ARM")
                # PIN: /Zi is added several other places. Disable /ZI to avoid the Command line warning D9025 : overriding '/Zi' with '/ZI' error
                # add_compile_options($<$<CONFIG:Debug>:/ZI>)     # Program Database for Edit And Continue
            endif()
            if(SMTG_USE_STATIC_CRT)
                add_compile_options($<$<CONFIG:Debug>:/MTd>)    # Runtime Library: /MTd = MultiThreaded Debug Runtime
                add_compile_options($<$<CONFIG:Release>:/MT>)   # Runtime Library: /MT  = MultiThreaded Runtime
            else()
                add_compile_options($<$<CONFIG:Debug>:/MDd>)    # Runtime Library: /MDd = MultiThreadedDLL Debug Runtime
                add_compile_options($<$<CONFIG:Release>:/MD>)   # Runtime Library: /MD  = MultiThreadedDLL Runtime
            endif()

            add_compile_options(/fp:fast)                   # Floating Point Model
            add_compile_options($<$<CONFIG:Release>:/Oi>)   # Enable Intrinsic Functions (Yes)
            add_compile_options($<$<CONFIG:Release>:/Ot>)   # Favor Size Or Speed (Favor fast code)
            add_compile_options($<$<CONFIG:Release>:/GF>)   # Enable String Pooling
            add_compile_options($<$<CONFIG:Release>:/EHa>)  # Enable C++ Exceptions
            add_compile_options($<$<CONFIG:Release>:/Oy>)   # Omit Frame Pointers
            #add_compile_options($<$<CONFIG:Release>:/Ox>)  # Optimization (/O2: Maximise Speed /0x: Full Optimization)
            set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
            set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
        endif()        
    endif()
endmacro()
