cmake_minimum_required(VERSION 3.10)

# 设置项目名称
project(MultNodeComProj)

# Find the Threads package
find_package(Threads REQUIRED)

# 获取所有源文件
file(GLOB_RECURSE ALL_SOURCES 
     "${PROJECT_SOURCE_DIR}/MultNodeCom/*.c")

# 添加可执行文件
add_executable(MultNodeComProj test/main.c ${ALL_SOURCES})

# Add the MultNodeCom directory
add_subdirectory(MultNodeCom)

# Link the MultNodeCom library
target_link_libraries(MultNodeComProj Threads::Threads)

# Specify the include directories
target_include_directories(MultNodeComProj PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/MultNodeCom")
target_include_directories(MultNodeComProj PUBLIC 
    "${PROJECT_BINARY_DIR}"
    "${PROJECT_SOURCE_DIR}/MultNodeCom"
    "${PROJECT_SOURCE_DIR}/MultNodeCom/3rd_part"
    "${PROJECT_SOURCE_DIR}/MultNodeCom/bus_device"
    "${PROJECT_SOURCE_DIR}/MultNodeCom/bus_driver"
    "${PROJECT_SOURCE_DIR}/MultNodeCom/bus_interface"
    # "${PROJECT_SOURCE_DIR}/MultNodeCom/routeCore"
    "${PROJECT_SOURCE_DIR}/MultNodeCom/utils"
)

target_compile_options(MultNodeComProj PRIVATE -g -O0)

# specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)
