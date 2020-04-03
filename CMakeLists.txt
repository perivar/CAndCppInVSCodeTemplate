cmake_minimum_required(VERSION 3.5)

# set verbose
set(CMAKE_VERBOSE_MAKEFILE ON)

# # call the CMakeLists.txt file in the freeverb directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vst2.x-plugins/freeverb-vst2.4")

# # call the CMakeLists.txt file in the minihost directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vst2.x-plugins/minihost")

# # call the CMakeLists.txt file in the again directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vst2.x-plugins/again")

# call the CMakeLists.txt file in the adelay directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vst2.x-plugins/adelay")

# call the CMakeLists.txt file in the minimal host directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vst2.x-plugins/minimal_vst2x_host")

# call the CMakeLists.txt file in the vstxsynth directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vst2.x-plugins/vstxsynth")

# call the CMakeLists.txt file in the Revitar directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/vst2.x-plugins/Revitar")

# call the CMakeLists.txt file in the vstgui.sf drawtest directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/include/VST2_SDK/vstgui.sf/drawtest")

# call the CMakeLists.txt file in the vstgui_360_rc2 tutorial directory
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/include/VST2_SDK/vstgui_360_rc2/vstgui/tutorial")

return()

# target path (SMTG_PLUGIN_TARGET_PATH)
set(SMTG_PLUGIN_TARGET_PATH "C:/Users/perner/My Projects/VstPlugins")

# folder to the cmake module files
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/VST3_SDK/cmake/modules")

# call the SMTG_VST3_SDK.cmake file
include(SMTG_VST3_SDK)

project(vstsdk)

setupPlatformToolset()

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/VST3_SDK")

# Here you can define where the VST 3 SDK is located
set(SDK_ROOT "${ROOT}")
set(public_sdk_SOURCE_DIR ${SDK_ROOT}/public.sdk)
set(pluginterfaces_SOURCE_DIR ${SDK_ROOT}/pluginterfaces)

# disable deprecated methods
set(VSTGUI_ENABLE_DEPRECATED_METHODS OFF)

# disable all vstgui tools and examples
set(VSTGUI_STANDALONE OFF)
set(VSTGUI_STANDALONE_EXAMPLES OFF)
set(VSTGUI_TOOLS OFF)
set(VSTGUI_DISABLE_UNITTESTS ON)

# disable validator
set(SMTG_RUN_VST_VALIDATOR OFF)

# Here you can define where the VSTGUI is located
if(SMTG_ADD_VSTGUI)
    set(VSTGUI_ROOT "${ROOT}")
    setupVstGuiSupport()
endif()

include_directories(${ROOT} ${SDK_ROOT})

set(SDK_IDE_LIBS_FOLDER FOLDER "Libraries")

#---Add base libraries---------------------------
set(VST_SDK TRUE) # used for Base module which provides only a subset of Base for VST-SDK
add_subdirectory(${SDK_ROOT}/pluginterfaces)
add_subdirectory(${SDK_ROOT}/base)
add_subdirectory(${SDK_ROOT}/public.sdk)
add_subdirectory(${SDK_ROOT}/public.sdk/source/vst/interappaudio)

# add VST examples
set(SDK_IDE_PLUGIN_EXAMPLES_FOLDER FOLDER "PlugInExamples")
add_subdirectory(${SDK_ROOT}/public.sdk/samples/vst/again)
add_subdirectory(${SDK_ROOT}/public.sdk/samples/vst/mda-vst3)
add_subdirectory(${SDK_ROOT}/public.sdk/samples/vst/hostchecker)

# Add hosting examples, it includes the validator
set(SDK_IDE_HOSTING_EXAMPLES_FOLDER FOLDER "HostingExamples")
add_subdirectory(${SDK_ROOT}/public.sdk/samples/vst-hosting/validator)
add_subdirectory(${SDK_ROOT}/public.sdk/samples/vst-hosting/editorhost)

return()

# https://github.com/steinbergmedia/vst3sdk/blob/master/CMakeLists.txt
cmake_minimum_required (VERSION 3.4.3)

set(SMTG_PLUGIN_TARGET_PATH "C:/Users/perner/My Projects/VstPlugins")

#-------------------------------------------------------------------------------
# Includes
#-------------------------------------------------------------------------------

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/include/cmake/modules")

include(SMTG_VST3_SDK)

# disable vst gui
# set(SMTG_ADD_VSTGUI 0)

#-------------------------------------------------------------------------------
# SDK Project
#-------------------------------------------------------------------------------
project(vstsdk)

setupPlatformToolset()

set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/include/VST3_SDK")

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

if(SMTG_ADD_VST3_HOSTING_SAMPLES)
    message("-- SMTG_ADD_VST3_HOSTING_SAMPLES is set.")
else()
    message("-- SMTG_ADD_VST3_HOSTING_SAMPLES is not set.")
endif()

if(SDK_ROOT)
    message("-- SDK_ROOT is set to : ${SDK_ROOT}")
endif()
