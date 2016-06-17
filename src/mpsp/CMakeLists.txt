cmake_minimum_required (VERSION 2.6)
project(mpsp)

aux_source_directory(. SRC_LIST)

add_executable(mpsp ${SRC_LIST})

if(WIN32)
    target_link_libraries(mpsp common wx mysqlcppconn)
elseif(UNIX)
    target_link_libraries(mpsp common wx mysqlcppconn pthread boost_system boost_filesystem boost_thread rt)
endif()

if(WIN32)
    set_target_properties(mpsp PROPERTIES COMPILE_FLAGS "/wd4996 /wd4819 /wd4715 /wd4251")
endif()

install(TARGETS mpsp
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib/static)