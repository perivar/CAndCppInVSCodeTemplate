
if(SMTG_ADD_VST3_HOSTING_SAMPLES)

    set(editorhost_sources
        ${SDK_ROOT}/public.sdk/source/vst/hosting/connectionproxy.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/connectionproxy.h
        ${SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/hostclasses.h
        ${SDK_ROOT}/public.sdk/source/vst/hosting/module.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/module.h
        ${SDK_ROOT}/public.sdk/source/vst/hosting/optional.h
        ${SDK_ROOT}/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/pluginterfacesupport.h
        ${SDK_ROOT}/public.sdk/source/vst/hosting/plugprovider.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/plugprovider.h
        ${SDK_ROOT}/public.sdk/source/vst/hosting/processdata.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/processdata.h
        ${SDK_ROOT}/public.sdk/source/vst/hosting/stringconvert.cpp
        ${SDK_ROOT}/public.sdk/source/vst/hosting/stringconvert.h
        ${SDK_ROOT}/public.sdk/source/vst/hosting/uid.h
        ${SDK_ROOT}/public.sdk/source/vst/vstinitiids.cpp
        source/editorhost.cpp
        source/editorhost.h
        source/platform/appinit.h
        source/platform/iapplication.h
        source/platform/iplatform.h
        source/platform/iwindow.h
        source/usediids.cpp
    )

    if(SMTG_MAC)
        set(editorhost_sources
            ${editorhost_sources}
            source/platform/mac/platform.mm
            source/platform/mac/window.h
            source/platform/mac/window.mm
            ${SDK_ROOT}/public.sdk/source/vst/hosting/module_mac.mm
        )
        set(editorhost_PLATFORM_LIBS
            "-framework Cocoa"
        )
        get_filename_component(InfoPlistFile "resource/Info.plist" ABSOLUTE)
        set(APP_PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
        )
    elseif(SMTG_LINUX)
        find_package(X11 REQUIRED)
        find_package(PkgConfig REQUIRED)
        pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
        pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
        include_directories(${X11_INCLUDE_DIR} ${GTKMM3_INCLUDE_DIRS} ${GTK3_INCLUDE_DIRS})
        set(editorhost_sources
            ${editorhost_sources}
            source/platform/linux/platform.cpp
            source/platform/linux/runloop.h
            source/platform/linux/runloop.cpp
            source/platform/linux/window.h
            source/platform/linux/window.cpp
            ${SDK_ROOT}/public.sdk/source/vst/hosting/module_linux.cpp
        )
        set(editorhost_PLATFORM_LIBS
            ${X11_LIBRARIES}
            ${GTK3_LIBRARIES}
            ${GTKMM3_LIBRARIES}
        )
        set(APP_PROPERTIES
        )
    elseif(SMTG_WIN)
        set(editorhost_sources
            ${editorhost_sources}
            source/platform/win32/platform.cpp
            source/platform/win32/window.h
            source/platform/win32/window.cpp
            ${SDK_ROOT}/public.sdk/source/vst/hosting/module_win32.cpp
        )
        set(editorhost_PLATFORM_LIBS
        )
        if(MSVC)
            set(APP_PROPERTIES
                LINK_FLAGS "/SUBSYSTEM:windows"
            )
        endif()
    endif()

    set(target editorhost)

    if(MINGW)
        # When the WIN32 property is set to true the executable when linked on Windows will be created with a WinMain() entry point instead of just main(). 
        # This makes it a GUI executable instead of a console application.
        # A WIN32 flag to add_executable means you're going to make it a Windows program, and provide a WinMain function.
        # This also results in no console window.
        # Therefore don't add the WIN32 property as the editorhost is a console application
        add_executable(${target} ${editorhost_sources})

        # together with -mwindows this make the entry point wWinMain instead of WinMain
        # https://gcc.gnu.org/onlinedocs/gcc/x86-Windows-Options.html   
        set_target_properties(${target} PROPERTIES LINK_FLAGS -municode)
    else()
        add_executable(${target} ${editorhost_sources})
    endif()

    set_target_properties(${target} PROPERTIES ${SDK_IDE_HOSTING_EXAMPLES_FOLDER})
    target_link_libraries(${target} PRIVATE base sdk ${editorhost_PLATFORM_LIBS})
    smtg_setup_universal_binary(${target})
    if (APP_PROPERTIES)
        set_target_properties(${target} PROPERTIES ${APP_PROPERTIES})
    endif()

endif()
