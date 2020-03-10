
if(ANDROID)
    add_subdirectory(adelay)
    return()
endif()

# PIN: disable samples by commenting the following two lines
include(AddSubDirectories)
add_subdirectories()

# only compile the one sample
# add_subdirectory("again")
