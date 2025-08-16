cmake_minimum_required(VERSION 3.1)
project(test_dep_2geom CXX C)
set(CMAKE_CXX_STANDARD 17)

option(2GEOM_AS_SUBPROJECT "include 2geom as subproject" OFF)

if (2GEOM_AS_SUBPROJECT)
    message("Using 2geom as subdirectory")
    set(2GEOM_BUILD_SHARED ON CACHE BOOL "Build 2geom shared version")
    add_subdirectory("../../" 2geom)
else()
    message("Using installed 2geom")
    find_package(2Geom REQUIRED)
endif()

add_library(my_lib SHARED my_lib.cpp)
add_executable(main main.cpp)
target_link_libraries(main my_lib)
target_link_libraries(my_lib PUBLIC 2Geom::2geom)

install(TARGETS
        main
        my_lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    )
