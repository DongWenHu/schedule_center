cmake_minimum_required (VERSION 2.6)

project(sche_center)

if(UNIX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
    set(CMAKE_BUILD_TYPE Debug)

    if(CMAKE_BUILD_TYPE STREQUAL Debug)
        message("Debug mode")
        add_definitions(-D_DEBUG)
    endif()
endif(UNIX)

if(WIN32)
    #此处根据自己boost安装目录修改
    include_directories(D:/boost_1_60_0)
	link_directories(D:/boost_1_60_0/lib32-msvc-12.0)
    add_definitions(-D_WIN32_WINNT=0x0501 -DWIN32_LEAN_AND_MEAN)

    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../lib/static)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../bin)
endif(WIN32)

set(CMAKE_INSTALL_PREFIX "${PROJECT_SOURCE_DIR}/../")
link_directories(${PROJECT_SOURCE_DIR}/../lib ${PROJECT_SOURCE_DIR}/../lib/static)

include_directories(${PROJECT_SOURCE_DIR}/../inc/wx 
    ${PROJECT_SOURCE_DIR}/../inc/common 
    ${PROJECT_SOURCE_DIR}/../inc/public
    ${PROJECT_SOURCE_DIR}/../ext/mysql/include
    )

add_subdirectory(common)
add_subdirectory(wx)
add_subdirectory(mpsp)