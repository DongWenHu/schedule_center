cmake_minimum_required (VERSION 2.6)
project(wx)

aux_source_directory(. WX_SRC_LIST)

add_library(wx STATIC ${WX_SRC_LIST})
target_link_libraries(wx common)

if(WIN32)
    set_target_properties(wx PROPERTIES COMPILE_FLAGS "/wd4996 /wd4819 /wd4715 /wd4251")
endif()

install(TARGETS wx
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)