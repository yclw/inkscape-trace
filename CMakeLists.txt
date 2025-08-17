cmake_minimum_required(VERSION 3.16)
project(ink)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 启用 pkg-config
find_package(PkgConfig REQUIRED)

### 使用 pkg-config 管理的库

# opencv4
pkg_check_modules(OPENCV REQUIRED opencv4)

### 手动配置的库（不支持 pkg-config）

# potrace
set(POTRACE_ROOT "/opt/homebrew/opt/potrace")
include_directories("${POTRACE_ROOT}/include")
link_directories("${POTRACE_ROOT}/lib")
set(POTRACE_LIBRARIES potrace)

### 应用 pkg-config 配置

# 添加编译标志
add_compile_options(
    ${OPENCV_CFLAGS_OTHER}
)

# 添加头文件路径
include_directories(
    ${OPENCV_INCLUDE_DIRS}
)

# 添加库路径
link_directories(
    ${OPENCV_LIBRARY_DIRS}
)

# 创建头文件库（仅包含头文件）
add_library(ink_bitmap INTERFACE)
target_include_directories(ink_bitmap INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}
    "${POTRACE_ROOT}/include"
)

# 收集所有必需的源文件
set(INK_SOURCES
    main.cpp
    trace/engine/potrace/potrace.cpp
    trace/imagemap/imagemap.cpp
    trace/filterset/filterset.cpp
    trace/filterset/quantize/quantize.cpp
    trace/trace.cpp
)

add_executable(ink ${INK_SOURCES})

target_link_libraries(ink
    ink_bitmap
    ${OPENCV_LIBRARIES}
    ${POTRACE_LIBRARIES}
)