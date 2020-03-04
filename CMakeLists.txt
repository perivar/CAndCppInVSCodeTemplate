# https://github.com/steinbergmedia/vst3sdk/blob/master/CMakeLists.txt
cmake_minimum_required (VERSION 3.4.3)

# add_compile_definitions(DEVELOPMENT)
set(SMTG_PLUGIN_TARGET_PATH "C:/Users/perner/My Projects/VstPlugins")

#-------------------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/cmake/modules")

include(SMTG_VST3_SDK)

# disable vst gui
set(SMTG_ADD_VSTGUI 0)

#-------------------------------------------------------------------------------
# SDK Project
#-------------------------------------------------------------------------------
project(vstsdk)

setupPlatformToolset()

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include")

# Here you can define where the VST 3 SDK is located
set(SDK_ROOT "${ROOT}")
set(public_sdk_SOURCE_DIR ${SDK_ROOT}/public.sdk)
set(pluginterfaces_SOURCE_DIR ${SDK_ROOT}/pluginterfaces)

# Here you can define where the VSTGUI is located
if(SMTG_ADD_VSTGUI)
    set(VSTGUI_ROOT "${ROOT}")
    setupVstGuiSupport()
endif()

include_directories(${ROOT} ${SDK_ROOT})

#-------------------------------------------------------------------------------
# From Here this is optional...

#-------------------------------------------------------------------------------
# CORE AUDIO SDK, AAX SDK
#-------------------------------------------------------------------------------
setupCoreAudioSupport()
setupAaxSupport()

#-------------------------------------------------------------------------------
# Projects
#-------------------------------------------------------------------------------
set(SDK_IDE_LIBS_FOLDER FOLDER "Libraries")

#---Add base libraries---------------------------
set(VST_SDK TRUE) # used for Base module which provides only a subset of Base for VST-SDK
add_subdirectory(${SDK_ROOT}/pluginterfaces)
add_subdirectory(${SDK_ROOT}/base)
add_subdirectory(${SDK_ROOT}/public.sdk)
add_subdirectory(${SDK_ROOT}/public.sdk/source/vst/interappaudio)

#---Add Wrappers (AU, AAX)-----------------------
if (SMTG_COREAUDIO_SDK_PATH)
    add_subdirectory(public.sdk/source/vst/auwrapper)
endif()

if(SMTG_AAX_SDK_PATH)
    add_subdirectory(public.sdk/source/vst/aaxwrapper)
    set_target_properties(aaxwrapper PROPERTIES ${SDK_IDE_LIBS_FOLDER})
endif()

#-------------------------------------------------------------------------------
# Here is added your own plugins folder location
#-------------------------------------------------------------------------------
set(SDK_IDE_MYPLUGINS_FOLDER FOLDER "MyPlugIns")
if(EXISTS ${SMTG_MYPLUGINS_SRC_PATH})
    set(SMTG_MYPLUGINS_BIN_PATH "${SMTG_MYPLUGINS_SRC_PATH}/build")
    add_subdirectory(${SMTG_MYPLUGINS_SRC_PATH} ${SMTG_MYPLUGINS_BIN_PATH})
endif()

#---Add VST3 examples (plug-ins and hosting)-----
if(SMTG_ADD_VST3_PLUGINS_SAMPLES)
    set(SDK_IDE_PLUGIN_EXAMPLES_FOLDER FOLDER "PlugInExamples")
    add_subdirectory(${SDK_ROOT}/public.sdk/samples/vst)
    add_subdirectory(${SDK_ROOT}/public.sdk/source/vst/auwrapper/again)
    add_subdirectory(${SDK_ROOT}/public.sdk/source/vst/auv3wrapper)
endif()

# Add hosting examples, it includes the validator
set(SDK_IDE_HOSTING_EXAMPLES_FOLDER FOLDER "HostingExamples")
add_subdirectory(${SDK_ROOT}/public.sdk/samples/vst-hosting)

#-------------------------------------------------------------------------------
# IDE sorting
#-------------------------------------------------------------------------------
set_target_properties(sdk PROPERTIES ${SDK_IDE_LIBS_FOLDER})
set_target_properties(base PROPERTIES ${SDK_IDE_LIBS_FOLDER})
set_target_properties(pluginterfaces PROPERTIES ${SDK_IDE_LIBS_FOLDER})
if(TARGET base_ios)
    set_target_properties(base_ios PROPERTIES ${SDK_IDE_LIBS_FOLDER})
endif()

if(SMTG_ADD_VSTGUI)
    set_target_properties(vstgui PROPERTIES ${SDK_IDE_LIBS_FOLDER})
    set_target_properties(vstgui_support PROPERTIES ${SDK_IDE_LIBS_FOLDER})
    set_target_properties(vstgui_uidescription PROPERTIES ${SDK_IDE_LIBS_FOLDER})

    if(TARGET vstgui_standalone)
        set_target_properties(vstgui_standalone PROPERTIES ${SDK_IDE_LIBS_FOLDER})
    endif()
endif()

# Changed by PIN: 04.03.2020
# https://blog.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
# these lines are not neccesary
# include(GenerateExportHeader)
# generate_export_header(sdk)

if(SMTG_ADD_VST3_HOSTING_SAMPLES)
    message("-- SMTG_ADD_VST3_HOSTING_SAMPLES is set.")
else()
    message("-- SMTG_ADD_VST3_HOSTING_SAMPLES is not set.")
endif()

if(SDK_ROOT)
    message("-- SDK_ROOT is set to : ${SDK_ROOT}")
endif()

