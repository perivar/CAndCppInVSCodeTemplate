
if(SMTG_ADD_VSTGUI)
    set(noteexpressiontext_sources
        source/plug.cpp
        source/plug.h
        source/plugcids.h
        source/plugcontroller.cpp
        source/plugcontroller.h
        source/plugentry.cpp
        source/plugparamids.h
        source/version.h
        resource/plug.uidesc
    )

    set(target noteexpressiontext)
    smtg_add_vst3plugin(${target} ${noteexpressiontext_sources})
    set_target_properties(${target} PROPERTIES ${SDK_IDE_PLUGIN_EXAMPLES_FOLDER})
    target_include_directories(${target} PRIVATE ${VSTGUI_ROOT}/vstgui4)

    # PIN 10.03.2020 - add libraries vstgui and the missing windows libraries
    if(MINGW)
        target_link_libraries(${target} PRIVATE base sdk vstgui_support vstgui windowscodecs)
    else()
        target_link_libraries(${target} PRIVATE base sdk vstgui_support)
    endif()

    smtg_add_vst3_resource(${target} "resource/plug.uidesc")

    if(SMTG_MAC)
        smtg_set_bundle(${target} INFOPLIST "${CMAKE_CURRENT_LIST_DIR}/resource/Info.plist" PREPROCESS)
    elseif(SMTG_WIN)
        target_sources(${target} PRIVATE resource/plug.rc)
    endif()
endif(SMTG_ADD_VSTGUI)
