cmake_minimum_required (VERSION 2.6)
project(common)

aux_source_directory(. COMMON_SRC_LIST)

add_library(common STATIC ${COMMON_SRC_LIST})

if(WIN32)
    set_target_properties(common PROPERTIES COMPILE_FLAGS "/wd4996 /wd4819 /wd4715 /wd4251")
endif()

install(TARGETS common
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)