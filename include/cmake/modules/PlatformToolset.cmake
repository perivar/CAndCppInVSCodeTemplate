
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
        # Changed by PIN: 25.02.2020
        # add_definitions(-D_UNICODE)

        # turn on all warnings
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

        # -mstackrealign Realign the stack at entry. 
        # On the Intel x86, the -mstackrealign option will generate an alternate prologue and epilogue 
        # that realigns the runtime stack if necessary. 
        # This supports mixing legacy codes that keep a 4-byte aligned stack with modern codes that keep a 
        # 16-byte stack for SSE compatibility. See also the attribute force_align_arg_pointer, applicable to individual functions.
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mstackrealign")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mstackrealign")
    
        set(CMAKE_POSITION_INDEPENDENT_CODE TRUE)
        set(CMAKE_CXX_STANDARD 17)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-multichar")
    
        if(ANDROID)
            set(CMAKE_ANDROID_STL_TYPE c++_static)
            link_libraries(dl)
        else()
            # link_libraries(stdc++fs pthread dl)
            # link_libraries(${VST_LIBS})
            # link_libraries(pthread)
        endif()

        # add_compile_options(/fp:fast)                   # Floating Point Model
        # add_compile_options($<$<CONFIG:Release>:/Oi>)   # Enable Intrinsic Functions (Yes)
        # add_compile_options($<$<CONFIG:Release>:/Ot>)   # Favor Size Or Speed (Favor fast code)
        # add_compile_options($<$<CONFIG:Release>:/GF>)   # Enable String Pooling
        # add_compile_options($<$<CONFIG:Release>:/EHa>)  # Enable C++ Exceptions
        # add_compile_options($<$<CONFIG:Release>:/Oy>)   # Omit Frame Pointers
        # #add_compile_options($<$<CONFIG:Release>:/Ox>)  # Optimization (/O2: Maximise Speed /0x: Full Optimization)
        # set(CMAKE_EXE_LINKER_FLAGS_DEBUG "${CMAKE_EXE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")
        # set(CMAKE_MODULE_LINKER_FLAGS_DEBUG "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} /SAFESEH:NO")

        # add subsystem windows to modules and shared libraries
        # SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--subsystem,windows")
        SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--subsystem,windows")
        SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--subsystem,windows")

    endif()
endmacro()
