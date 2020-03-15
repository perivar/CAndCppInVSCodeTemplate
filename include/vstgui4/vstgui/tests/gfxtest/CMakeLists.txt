##########################################################################################
# VSTGUI GFXTest Application
##########################################################################################
set(target GFXTest)

set(${target}_sources
  "source/app.cpp"
  "source/drawdevicetests.cpp"
  "source/drawdevicetests.h"
)

##########################################################################################
if(CMAKE_HOST_APPLE)
	set(BUNDLE_PKG_INFO
    "resource/PkgInfo"
	)
	set(gfxtest_resources
		"resource/Window.uidesc"
		"resource/DrawDeviceTests.uidesc"
	)
	set_source_files_properties(${BUNDLE_PKG_INFO} PROPERTIES
		MACOSX_PACKAGE_LOCATION "."
	)
  set_source_files_properties(${gfxtest_resources} PROPERTIES
    MACOSX_PACKAGE_LOCATION "Resources"
  )
  set(gfxtest_resources ${gfxtest_resources} ${BUNDLE_PKG_INFO})
  
	set(${target}_PLATFORM_LIBS
		"-framework Cocoa"
		"-framework OpenGL"
		"-framework QuartzCore"
		"-framework Accelerate"
	)
	get_filename_component(InfoPlistFile "resource/Info.plist" ABSOLUTE)
	set(APP_PROPERTIES
		MACOSX_BUNDLE TRUE
		MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
		OUTPUT_NAME "GFXTest"
	)
endif()

##########################################################################################
# Changed by PIN: 07.03.2020
# if(MSVC)
#   set(gfxtest_resources
#     "resource/gfxtest.rc"
#   )
#   set(APP_PROPERTIES
#     LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
#   )
# endif()

if(SMTG_WIN)
  set(gfxtest_resources
    "resource/gfxtest.rc"
  )
endif()

##########################################################################################
include_directories(../../../)
add_executable(${target}
  ${${target}_sources}
  ${gfxtest_resources}
)
target_link_libraries(${target}
	vstgui
  vstgui_uidescription
  vstgui_standalone
	${${target}_PLATFORM_LIBS}
)

# PIN 10.03.2020 - change version to 17
vstgui_set_cxx_version(${target} 17)

set_target_properties(${target} PROPERTIES ${APP_PROPERTIES} FOLDER Tests)
target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})
