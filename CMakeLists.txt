cmake_minimum_required(VERSION 3.16)
project(ink)

# 设置 C++ 标准（2geom 需要 C++20）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 pkg-config
find_package(PkgConfig REQUIRED)

### 使用 pkg-config 管理的库

# glib-2.0
pkg_check_modules(GLIB REQUIRED glib-2.0)

# opencv4
pkg_check_modules(OPENCV REQUIRED opencv4)

### 手动配置的库（不支持 pkg-config）

# 2geom
set(GEOM2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/lib2geom")

# 设置2geom构建选项（避免构建不需要的组件）
set(2GEOM_TESTING OFF CACHE BOOL "Build 2geom tests" FORCE)
set(2GEOM_BUILD_SHARED OFF CACHE BOOL "Build 2geom as shared library" FORCE)

# 将2geom作为子项目添加
add_subdirectory(lib2geom)

# 使用2geom目标和头文件
set(GEOM2_LIBRARIES 2geom)
include_directories("${GEOM2_ROOT}/include")

# potrace
set(POTRACE_ROOT "/opt/homebrew/opt/potrace")
include_directories("${POTRACE_ROOT}/include")
link_directories("${POTRACE_ROOT}/lib")
set(POTRACE_LIBRARIES potrace)

# boost
set(BOOST_ROOT "/opt/homebrew/opt/boost")
include_directories("${BOOST_ROOT}/include")
link_directories("${BOOST_ROOT}/lib")
set(BOOST_LIBRARIES boost_system boost_filesystem)

### 应用 pkg-config 配置

# 添加编译标志
add_compile_options(
    ${GLIB_CFLAGS_OTHER}
    ${OPENCV_CFLAGS_OTHER}
)

# 添加头文件路径
include_directories(
    ${GLIB_INCLUDE_DIRS}
    ${OPENCV_INCLUDE_DIRS}
)

# 添加库路径
link_directories(
    ${GLIB_LIBRARY_DIRS}
    ${OPENCV_LIBRARY_DIRS}
)

# 创建头文件库（仅包含头文件）
add_library(ink_bitmap INTERFACE)
target_include_directories(ink_bitmap INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${POTRACE_ROOT}/include"
)

# GSL库（GNU Scientific Library）
set(GSL_ROOT "/opt/homebrew/opt/gsl")
include_directories("${GSL_ROOT}/include")
link_directories("${GSL_ROOT}/lib")
set(GSL_LIBRARIES gsl gslcblas)

# double-conversion库
set(DOUBLE_CONVERSION_ROOT "/opt/homebrew/opt/double-conversion")
include_directories("${DOUBLE_CONVERSION_ROOT}/include")
link_directories("${DOUBLE_CONVERSION_ROOT}/lib")
set(DOUBLE_CONVERSION_LIBRARIES double-conversion)

# 收集所有必需的源文件
set(INK_SOURCES
    main.cpp
    trace/engine/potrace/potrace.cpp
    trace/imagemap/imagemap.cpp
    trace/filterset/filterset.cpp
    trace/filterset/quantize/quantize.cpp
    svg/path-string.cpp
    svg/svg-path.cpp
    svg/svg-writer.cpp
    svg/svg-length.cpp
    trace/trace.cpp
)

add_executable(ink ${INK_SOURCES})

target_link_libraries(ink
    ink_bitmap
    ${GLIB_LIBRARIES}
    ${OPENCV_LIBRARIES}
    ${POTRACE_LIBRARIES}
    ${BOOST_LIBRARIES}
    ${GSL_LIBRARIES}
    ${DOUBLE_CONVERSION_LIBRARIES}
    ${GEOM2_LIBRARIES}
)