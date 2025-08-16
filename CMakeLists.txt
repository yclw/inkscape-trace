cmake_minimum_required(VERSION 3.16)
project(ink)

# 设置 C++ 标准（2geom 需要 C++20）
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 pkg-config
find_package(PkgConfig REQUIRED)

### 使用 pkg-config 管理的库

# gtkmm-4.0 (包含 gdkmm)
pkg_check_modules(GTKMM REQUIRED gtkmm-4.0)

# glib-2.0
pkg_check_modules(GLIB REQUIRED glib-2.0)

# glibmm-2.4
pkg_check_modules(GLIBMM REQUIRED glibmm-2.4)

# opencv4
pkg_check_modules(OPENCV REQUIRED opencv4)

### 手动配置的库（不支持 pkg-config）

# 2geom
set(GEOM2_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/2geom")

# 设置2geom构建选项（避免构建不需要的组件）
set(2GEOM_TESTING OFF CACHE BOOL "Build 2geom tests" FORCE)
set(2GEOM_BUILD_SHARED OFF CACHE BOOL "Build 2geom as shared library" FORCE)

# 将2geom作为子项目添加
add_subdirectory(2geom)

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
    ${GTKMM_CFLAGS_OTHER}
    ${GLIB_CFLAGS_OTHER}
    ${GLIBMM_CFLAGS_OTHER}
    ${OPENCV_CFLAGS_OTHER}
)

# 添加头文件路径
include_directories(
    ${GTKMM_INCLUDE_DIRS}
    ${GLIB_INCLUDE_DIRS}
    ${GLIBMM_INCLUDE_DIRS}
    ${OPENCV_INCLUDE_DIRS}
)

# 添加库路径
link_directories(
    ${GTKMM_LIBRARY_DIRS}
    ${GLIB_LIBRARY_DIRS}
    ${GLIBMM_LIBRARY_DIRS}
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
    ppotrace.cpp
    quantize.cpp
    cielab.cpp
    imagemap.cpp
    imagemap-gdk.cpp
    filterset.cpp
    svg/path-string.cpp
    svg/stringstream.cpp
    svg/svg-length.cpp
    svg/strip-trailing-zeros.cpp
    svg/svg-path.cpp
    util/units.cpp
    util-string/string-convert.cpp
)

# 创建主程序可执行文件
add_executable(ink ${INK_SOURCES})

# 链接所有必要的库
target_link_libraries(ink 
    ink_bitmap
    ${GTKMM_LIBRARIES}
    ${GLIB_LIBRARIES}
    ${GLIBMM_LIBRARIES}
    ${OPENCV_LIBRARIES}
    ${POTRACE_LIBRARIES}
    ${BOOST_LIBRARIES}
    ${GSL_LIBRARIES}
    ${DOUBLE_CONVERSION_LIBRARIES}
    ${GEOM2_LIBRARIES}
)