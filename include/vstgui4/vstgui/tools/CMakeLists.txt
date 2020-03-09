set(VSTGUI_TOOLS_FOLDER FOLDER "VSTGUI Tools")

# PIN 09.03.2020 - disabled the vst gui tools
if(VSTGUI_STANDALONE AND NOT (WIN32 AND CMAKE_SIZEOF_VOID_P EQUAL 4)) 
    # add_subdirectory(imagestitcher)

endif()
# add_subdirectory(uidesccompressor)
