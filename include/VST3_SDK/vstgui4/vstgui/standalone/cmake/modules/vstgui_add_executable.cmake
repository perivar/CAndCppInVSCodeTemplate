###########################################################################################
get_filename_component(PkgInfoResource "${CMAKE_CURRENT_SOURCE_DIR}/cmake/resources/PkgInfo" ABSOLUTE)

###########################################################################################
if(LINUX)
  pkg_check_modules(GTKMM3 REQUIRED gtkmm-3.0)
endif(LINUX)

###########################################################################################
function(vstgui_add_executable target sources)

  # Changed by PIN: 04.03.2020
  if(MSVC)
    add_executable(${target} WIN32 ${sources})
    set_target_properties(${target} PROPERTIES LINK_FLAGS "/INCLUDE:wWinMain")
    get_target_property(OUTPUTDIR ${target} RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUTDIR}/${target}")
  endif(MSVC)

  if(MINGW)
    add_executable(${target} WIN32 ${sources})
    get_target_property(OUTPUTDIR ${target} RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUTDIR}/${target}")

    # PIN ensure that the find library also finds windows dll's 
    # the standard find_library` command does no longer consider .dll files to be linkable libraries. 
    # all dynamic link libraries are expected to provide separate .dll.a or .lib import libraries.
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".dll" ".dll.a" ".a" ".lib")
    # set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH} ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES}")

    find_library(UUID_FRAMEWORK uuid HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)                      # IID_<> variables
    # find_library(FREEGLUT_FRAMEWORK freeglut HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)              # Freeglut is dynamically linked 
    # find_library(OPENGL32_FRAMEWORK opengl32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)              # OpenGL Library
    # find_library(GLU32_FRAMEWORK glu32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)                    # OpenGL Utility Library
    # find_library(GLEW32_FRAMEWORK glew32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)                  # OpenGL Extension Wrangler Library 
    # find_library(GDI32_FRAMEWORK gdi32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)                    # OpenGL pixel format functions & SwapBuffers
    # find_library(DWMAPI_FRAMEWORK dwmapi HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)                  # Desktop Window Manager (DWM)
    # find_library(D2D1_FRAMEWORK d2d1 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)                      # Direct2D library 
    # find_library(DWRITE_FRAMEWORK dwrite HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)                  # DirectX Typography Services
    # find_library(COMCTL32_FRAMEWORK comctl32 HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)              # The Common Controls Library - provider of the more interesting window controls
    # find_library(SHLWAPI_FRAMEWORK shlwapi HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)                # Shell Light-Weight Application Programming Interface 
    # find_library(WINDOWSCODECS_FRAMEWORK windowscodecs HINTS ${CMAKE_C_IMPLICIT_LINK_DIRECTORIES} REQUIRED)

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

    message(STATUS "Linking with libraries: 
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

    # ensure the vst gui sources finds eachother modules
    target_link_libraries(${target}
      vstgui
      vstgui_uidescription
      vstgui_standalone
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
  endif(MINGW)

  if(LINUX)
    add_executable(${target} ${sources})
    get_target_property(OUTPUTDIR ${target} RUNTIME_OUTPUT_DIRECTORY)
    set_target_properties(${target} PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUTPUTDIR}/${target}")
    set(PLATFORM_LIBRARIES ${GTKMM3_LIBRARIES})
  endif(LINUX)

  if(CMAKE_HOST_APPLE)
    set_source_files_properties(${PkgInfoResource} PROPERTIES
      MACOSX_PACKAGE_LOCATION "."
    )
    add_executable(${target} ${sources} ${PkgInfoResource})
    set(PLATFORM_LIBRARIES
      "-framework Cocoa"
      "-framework OpenGL"
      "-framework QuartzCore"
      "-framework Accelerate"
    )
    set_target_properties(${target} PROPERTIES
      MACOSX_BUNDLE TRUE
      XCODE_ATTRIBUTE_DEBUG_INFORMATION_FORMAT $<$<CONFIG:Debug>:dwarf>$<$<NOT:$<CONFIG:Debug>>:dwarf-with-dsym>
      XCODE_ATTRIBUTE_DEPLOYMENT_POSTPROCESSING $<$<CONFIG:Debug>:NO>$<$<NOT:$<CONFIG:Debug>>:YES>
      OUTPUT_NAME "${target}"
    )
  endif(CMAKE_HOST_APPLE)

  target_link_libraries(${target}
    vstgui
    vstgui_uidescription
    vstgui_standalone
    ${PLATFORM_LIBRARIES}
  )
  target_compile_definitions(${target} ${VSTGUI_COMPILE_DEFINITIONS})

  if(ARGC GREATER 2)
    vstgui_add_resources(${target} "${ARGV2}")
    message(DEPRECATION "Please use vstgui_add_resources to add resources to an executable now.")
  endif()

endfunction()

###########################################################################################
function(vstgui_add_resources target resources)
  set(destination "Resources")
  if(ARGC GREATER 2)
    set(destination "${destination}/${ARGV2}")
  endif()
  if(CMAKE_HOST_APPLE)
    set_source_files_properties(${resources} PROPERTIES
      MACOSX_PACKAGE_LOCATION "${destination}"
    )
    target_sources(${target} PRIVATE ${resources})
  else()
    get_target_property(OUTPUTDIR ${target} RUNTIME_OUTPUT_DIRECTORY)
    set(destination "${OUTPUTDIR}/${destination}")
    if(NOT EXISTS ${destination})
      add_custom_command(TARGET ${target} PRE_BUILD
          COMMAND ${CMAKE_COMMAND} -E make_directory
          "${destination}"
      )
    endif()
    foreach(resource ${resources})
      add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_LIST_DIR}/${resource}"
        "${destination}"
      )
    endforeach(resource ${resources})
  endif()  
endfunction()

###########################################################################################
function(vstgui_set_target_infoplist target infoplist)
  if(CMAKE_HOST_APPLE)
    get_filename_component(InfoPlistFile "${infoplist}" ABSOLUTE)
    set_target_properties(${target} PROPERTIES
      MACOSX_BUNDLE_INFO_PLIST ${InfoPlistFile}
    )
  endif(CMAKE_HOST_APPLE)
endfunction()

###########################################################################################
function(vstgui_set_target_rcfile target rcfile)
  # Changed by PIN: 07.03.2020
  if(MINGW)
    # check if the rcfile is empty to avoid windres errors
    file(READ ${rcfile} rcfile_content)
    if(NOT ${rcfile_content} STREQUAL "")
      message("Using resource file: ${rcfile}")

      # call windres with the resource fil
      target_sources(${target} PRIVATE ${rcfile})
    endif()
  else()
    target_sources(${target} PRIVATE ${rcfile})
  endif(MINGW)

endfunction()
