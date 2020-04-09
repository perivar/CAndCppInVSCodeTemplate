
#-------------------------------------------------------------------------------
# Exported Symbols
#-------------------------------------------------------------------------------

function(smtg_set_exported_symbols target exported_symbols_file)
    if(MSVC)
        set_target_properties(${target} PROPERTIES LINK_FLAGS "/DEF:\"${exported_symbols_file}\"")
    elseif(XCODE)
        set_target_properties(${target} PROPERTIES XCODE_ATTRIBUTE_EXPORTED_SYMBOLS_FILE "${exported_symbols_file}")
    else()
        # PIN: 30.03.2020 - this is only used by MacOS   
        if(NOT MINGW)
            set_target_properties(${target} PROPERTIES LINK_FLAGS "-exported_symbols_list \"${exported_symbols_file}\"")
        endif()
    endif()
endfunction()
