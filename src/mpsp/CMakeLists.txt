cmake_minimum_required (VERSION 2.6)
project(mpsp)

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/../../bin)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../inc/wx 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc/common 
    ${CMAKE_CURRENT_SOURCE_DIR}/../../inc/public
    ${CMAKE_CURRENT_SOURCE_DIR}/../../ext/mysql/include
    )
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/../../lib
    ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/static
    )
if(WIN32)
    include_directories(D:/boost_1_60_0 F:/boost_1_60_0)
	link_directories(D:/boost_1_60_0/lib32-msvc-12.0)
    add_definitions(-D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN)
endif()

aux_source_directory(. SRC_LIST)

add_executable(mpsp ${SRC_LIST})

target_link_libraries(mpsp common wx mysqlcppconn)

if(WIN32)
    set_target_properties(mpsp PROPERTIES COMPILE_FLAGS "/wd4996 /wd4819 /wd4715 /wd4251")
endif()

install(TARGETS mpsp
        RUNTIME DESTINATION ../bin
        LIBRARY DESTINATION ../lib
        ARCHIVE DESTINATION ../lib/static)