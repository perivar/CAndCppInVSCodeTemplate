set(TargetName ImageStitcher)

set(${TargetName}_sources
  Readme.md
  source/app.cpp
  source/document.cpp
  source/document.h
  source/documentcontroller.cpp
  source/documentcontroller.h
  source/imageframesview.cpp
  source/imageframesview.h
  source/startupcontroller.cpp
  source/startupcontroller.h
)

set(${TargetName}_RESOURCES
  "resource/Window.uidesc"
  "resource/StartupWindow.uidesc"
)

# Changed by PIN: 07.03.2020
if(MSVC)
  SET(${TargetName}_SOURCE_FILES
    ${${TargetName}_SOURCE_FILES}
    ${WIN32_PLUG_EXPORT_DEF}
    "resource/imagestitcher.rc"
  )
  set(APP_PROPERTIES
    LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain"
  )
  set(${TargetName}_RESOURCES ${${TargetName}_RESOURCES}
    "resource/imagestitcher.rc"
  )
endif()

if(MINGW)
  SET(${TargetName}_SOURCE_FILES
    ${${TargetName}_SOURCE_FILES}
    ${WIN32_PLUG_EXPORT_DEF}
    "resource/imagestitcher.rc"
  )
  set(${TargetName}_RESOURCES ${${TargetName}_RESOURCES}
    "resource/imagestitcher.rc"
  )
endif()

vstgui_add_executable(${TargetName} "${${TargetName}_sources}")

# PIN 10.03.2020 - add libraries vstgui and windowscodecs
if(MINGW)
  target_link_libraries(${TargetName} vstgui windowscodecs)
endif()

vstgui_add_resources(${TargetName} "${${TargetName}_RESOURCES}")
vstgui_set_target_infoplist(${TargetName} "resource/Info.plist")
vstgui_set_target_rcfile(${TargetName} "resource/imagestitcher.rc")

# PIN 10.03.2020 - change version to 17
vstgui_set_cxx_version(${TargetName} 17)

target_include_directories(${TargetName} PRIVATE ../../../)
set_target_properties(${TargetName} PROPERTIES ${APP_PROPERTIES} ${VSTGUI_TOOLS_FOLDER})

