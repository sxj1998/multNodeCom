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
    "${PROJECT_SOURCE_DIR}/MultNodeCom/bus_core"
    # "${PROJECT_SOURCE_DIR}/MultNodeCom/routeCore"
    "${PROJECT_SOURCE_DIR}/MultNodeCom/utils"
)

target_compile_options(MultNodeComProj PRIVATE -g -O0)

# specify the C standard
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED True)


# 获取test目录中的所有.c文件
file(GLOB TEST_SOURCES "${PROJECT_SOURCE_DIR}/test/normal_test/*.c")

# 为每个test目录中的.c文件创建一个可执行文件
foreach(TEST_SOURCE ${TEST_SOURCES})
    # 获取文件名，不带路径和扩展名
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    # 创建可执行文件
    add_executable(${TEST_NAME} ${TEST_SOURCE} ${ALL_SOURCES})
    
    # 链接库
    target_link_libraries(${TEST_NAME} MultNodeCom Threads::Threads)
    
    # 指定include目录
    target_include_directories(${TEST_NAME} PUBLIC "${PROJECT_BINARY_DIR}" "${PROJECT_SOURCE_DIR}/MultNodeCom")
    
    # 编译选项
    target_compile_options(${TEST_NAME} PRIVATE -g -O0 -DCPPUTEST)
endforeach()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 获取test/cppUTest目录中的所有.cpp文件
file(GLOB CPPUTEST_SOURCES "${PROJECT_SOURCE_DIR}/test/cppUTest/*.cpp")
# 查找 CppUTest 库
find_library(CPPUTEST_LIBRARY CppUTest)
find_library(CPPUTESTEXT_LIBRARY CppUTestExt)

# 为每个test/cppUTest目录中的.cpp文件创建一个可执行文件
foreach(CPPUTEST_SOURCE ${CPPUTEST_SOURCES})
    # 获取文件名，不带路径和扩展名
    get_filename_component(TEST_NAME ${CPPUTEST_SOURCE} NAME_WE)
    
    # 创建可执行文件
    add_executable(${TEST_NAME} ${CPPUTEST_SOURCE} ${ALL_SOURCES})
    
    # 链接库
    target_link_libraries(${TEST_NAME} MultNodeCom Threads::Threads ${CPPUTEST_LIBRARY} ${CPPUTESTEXT_LIBRARY})

    # 指定包含目录
    target_include_directories(${TEST_NAME} PUBLIC 
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/MultNodeCom"
    )
    
    # 编译选项
    target_compile_options(${TEST_NAME} PRIVATE -g -O0 -DCPPUTEST)
endforeach()
