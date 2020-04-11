set(VSTGUI_STANDALONE_EXAMPLES_FOLDER FOLDER "VSTGUI Standalone Examples")

# Changed by PIN: 08.04.2020
if (MSVC)
  # set(APP_PROPERTIES  LINK_FLAGS "/SUBSYSTEM:windows" )
  # set(APP_PROPERTIES  LINK_FLAGS "/SUBSYSTEM:windows /INCLUDE:wWinMain" )  

  # this allows the examples to compile, but crashes when run
  # set(APP_PROPERTIES	LINK_FLAGS "/SUBSYSTEM:console /ENTRY:wWinMainCRTStartup" )
endif()

# PIN 08.04.2020
if(NOT MSVC_TOOLSET_VERSION OR MSVC_TOOLSET_VERSION VERSION_GREATER 140)
  add_subdirectory(mandelbrot)
  add_subdirectory(minesweeper)
endif()
add_subdirectory(simple_standalone)
add_subdirectory(standalone)
