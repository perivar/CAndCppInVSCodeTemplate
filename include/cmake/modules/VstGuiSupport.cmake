
macro(setupVstGuiSupport)
    # PIN: enabled the unit tests
    set(VSTGUI_DISABLE_UNITTESTS OFF)
    # set(VSTGUI_DISABLE_UNITTESTS 1)
    # PIN: enabled the standalone examples
    set(VSTGUI_STANDALONE_EXAMPLES ON)
    # set(VSTGUI_STANDALONE_EXAMPLES OFF)
    if(SMTG_BUILD_UNIVERSAL_BINARY)
        set(VSTGUI_STANDALONE OFF)
        set(VSTGUI_TOOLS OFF)
    elseif(SMTG_WIN AND CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(VSTGUI_STANDALONE OFF)
        set(VSTGUI_TOOLS OFF)
    else()
        set(VSTGUI_STANDALONE ON)
    endif()
    
    add_subdirectory(${VSTGUI_ROOT}/vstgui4/vstgui)

    set(VST3_VSTGUI_SOURCES
        ${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3groupcontroller.cpp
        ${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3groupcontroller.h
        ${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3padcontroller.cpp
        ${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3padcontroller.h
        ${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3editor.cpp
        ${VSTGUI_ROOT}/vstgui4/vstgui/plugin-bindings/vst3editor.h
        ${SDK_ROOT}/public.sdk/source/vst/vstguieditor.cpp
        )
    add_library(vstgui_support STATIC ${VST3_VSTGUI_SOURCES})
    target_compile_definitions(vstgui_support PUBLIC $<$<CONFIG:Debug>:VSTGUI_LIVE_EDITING>)
    if(VSTGUI_ENABLE_DEPRECATED_METHODS)
        target_compile_definitions(vstgui_support PUBLIC VSTGUI_ENABLE_DEPRECATED_METHODS=1)
    else()
        target_compile_definitions(vstgui_support PUBLIC VSTGUI_ENABLE_DEPRECATED_METHODS=0)
    endif()
    target_include_directories(vstgui_support PUBLIC ${VSTGUI_ROOT}/vstgui4)
    target_link_libraries(vstgui_support PRIVATE vstgui_uidescription)
    smtg_setup_universal_binary(vstgui_support)
    smtg_setup_universal_binary(vstgui)
    smtg_setup_universal_binary(vstgui_uidescription)
    if(SMTG_MAC)
        if(XCODE)
            target_link_libraries(vstgui_support PRIVATE "-framework Cocoa" "-framework OpenGL" "-framework Accelerate" "-framework QuartzCore" "-framework Carbon")
        else()
            find_library(COREFOUNDATION_FRAMEWORK CoreFoundation)
            find_library(COCOA_FRAMEWORK Cocoa)
            find_library(OPENGL_FRAMEWORK OpenGL)
            find_library(ACCELERATE_FRAMEWORK Accelerate)
            find_library(QUARTZCORE_FRAMEWORK QuartzCore)
            find_library(CARBON_FRAMEWORK Carbon)
            target_link_libraries(vstgui_support PRIVATE ${COREFOUNDATION_FRAMEWORK} ${COCOA_FRAMEWORK} ${OPENGL_FRAMEWORK} ${ACCELERATE_FRAMEWORK} ${QUARTZCORE_FRAMEWORK} ${CARBON_FRAMEWORK})
        endif()
    endif()

    if(SMTG_WIN)
        find_library(UUID_FRAMEWORK uuid HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})                      # IID_<> variables
        # find_library(FREEGLUT_FRAMEWORK freeglut HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})              # Freeglut is dynamically linked 
        # find_library(OPENGL32_FRAMEWORK opengl32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})              # OpenGL Library
        # find_library(GLU32_FRAMEWORK glu32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})                    # OpenGL Utility Library
        # find_library(GLEW32_FRAMEWORK glew32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})                  # OpenGL Extension Wrangler Library 
        # find_library(GDI32_FRAMEWORK gdi32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})                    # OpenGL pixel format functions & SwapBuffers
        # find_library(DWMAPI_FRAMEWORK dwmapi HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})                  # Desktop Window Manager (DWM)
        # find_library(D2D1_FRAMEWORK d2d1 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})                      # Direct2D library 
        # find_library(DWRITE_FRAMEWORK dwrite HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})                  # DirectX Typography Services
        # find_library(COMCTL32_FRAMEWORK comctl32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})              # The Common Controls Library - provider of the more interesting window controls
        # find_library(SHLWAPI_FRAMEWORK shlwapi HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})                # Shell Light-Weight Application Programming Interface 
        # find_library(WINDOWSCODECS_FRAMEWORK windowscodecs HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES})

        # find_library(UUID_FRAMEWORK uuid REQUIRED)                      # IID_<> variables
        find_library(FREEGLUT_FRAMEWORK freeglut REQUIRED)              # Freeglut is dynamically linked 
        find_library(OPENGL32_FRAMEWORK opengl32 REQUIRED)              # OpenGL Library
        find_library(GLU32_FRAMEWORK glu32 REQUIRED)                    # OpenGL Utility Library
        find_library(GLEW32_FRAMEWORK glew32 REQUIRED)                  # OpenGL Extension Wrangler Library 
        find_library(GDI32_FRAMEWORK gdi32 REQUIRED)                    # OpenGL pixel format functions & SwapBuffers
        find_library(DWMAPI_FRAMEWORK dwmapi REQUIRED)                  # Desktop Window Manager (DWM)
        find_library(D2D1_FRAMEWORK d2d1 REQUIRED)                      # Direct2D library 
        find_library(DWRITE_FRAMEWORK dwrite REQUIRED)                  # DirectX Typography Services
        find_library(COMCTL32_FRAMEWORK comctl32 REQUIRED)              # The Common Controls Library - provider of the more interesting window controls
        find_library(SHLWAPI_FRAMEWORK shlwapi REQUIRED)                # Shell Light-Weight Application Programming Interface 
        find_library(WINDOWSCODECS_FRAMEWORK windowscodecs REQUIRED)

        message(STATUS "Windows libraries found: 
            ${UUID_FRAMEWORK}
            ${FREEGLUT_FRAMEWORK}
            ${OPENGL32_FRAMEWORK}
            ${GLU32_FRAMEWORK}
            ${GLEW32_FRAMEWORK}
            ${GDI32_FRAMEWORK}
            ${DWMAPI_FRAMEWORK}
            ${D2D1_FRAMEWORK}
            ${DWRITE_FRAMEWORK}
            ${COMCTL32_FRAMEWORK}
            ${SHLWAPI_FRAMEWORK}
            ${WINDOWSCODECS_FRAMEWORK}
        " )

        target_link_libraries(vstgui_support PRIVATE 
            ${UUID_FRAMEWORK}
            ${FREEGLUT_FRAMEWORK}
            ${OPENGL32_FRAMEWORK}
            ${GLU32_FRAMEWORK}
            ${GLEW32_FRAMEWORK}
            ${GDI32_FRAMEWORK}
            ${DWMAPI_FRAMEWORK}
            ${D2D1_FRAMEWORK}
            ${DWRITE_FRAMEWORK}
            ${COMCTL32_FRAMEWORK}
            ${SHLWAPI_FRAMEWORK}
            ${WINDOWSCODECS_FRAMEWORK}        
        )

        # link_libraries(
        #     uuid          # IID_<> variables

        #     freeglut      # Freeglut is dynamically linked 
        #     opengl32      # OpenGL Library
        #     glu32         # OpenGL Utility Library
        #     glew32        # OpenGL Extension Wrangler Library 
        #     gdi32         # OpenGL pixel format functions & SwapBuffers
        #     dwmapi        # Desktop Window Manager (DWM) 

        #     d2d1          # Direct2D library 
        #     dwrite        # DirectX Typography Services

        #     comctl32      # The Common Controls Library - provider of the more interesting window controls
        #     shlwapi       # Shell Light-Weight Application Programming Interface 
        #     windowscodecs        
        # )
        endif()

    if(SMTG_WIN AND SMTG_CREATE_BUNDLE_FOR_WINDOWS)
        target_compile_definitions(vstgui_support PUBLIC SMTG_MODULE_IS_BUNDLE=1)
        target_sources(vstgui_support PRIVATE
            ${SDK_ROOT}/public.sdk/source/vst/vstgui_win32_bundle_support.cpp
            ${SDK_ROOT}/public.sdk/source/vst/vstgui_win32_bundle_support.h
        )
    endif()
    message(STATUS "VSTGUI_ROOT is set to : " ${VSTGUI_ROOT})
endmacro()
