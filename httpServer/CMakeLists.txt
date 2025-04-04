cmake_minimum_required(VERSION 2.5)
project(httpServer)

# 设置动态库的路径
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
# 设置可执行文件输出路径
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

# 设置调试信息和C++17标准
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++17 -fPIC")

# 查找依赖包
find_package(MySQL QUIET)
if(NOT MySQL_FOUND)
    find_library(MySQL_LIBRARIES NAMES mysqlclient)
    find_path(MySQL_INCLUDE_DIRS NAMES mysql.h PATH_SUFFIXES mysql)
endif()
if(NOT MySQL_LIBRARIES OR NOT MySQL_INCLUDE_DIRS)
    message(FATAL_ERROR "MySQL development files not found!")
endif()
find_package(PkgConfig REQUIRED)
pkg_check_modules(JSONCPP jsoncpp)
pkg_check_modules(TINYXML2 REQUIRED tinyxml2)

# 添加头文件路径
include_directories(
    ${MySQL_INCLUDE_DIRS} 
    ${JSONCPP_INCLUDE_DIRS}
    ${TINYXML2_INCLUDE_DIRS}
)

# 查找源文件
aux_source_directory(. SRC_LIST)

# 从SRC_LIST中移除http_server_test.cc
list(REMOVE_ITEM SRC_LIST "./http_server_test.cc")

# 添加HTTP相关源文件
set(HTTP_SRC
    HttpContext.cc
    HttpResponse.cc
    RouterHandler.cc
)

# 将HTTP源文件添加到库源文件列表
list(APPEND SRC_LIST ${HTTP_SRC})

# 编译生成动态库
add_library(httpServer SHARED ${SRC_LIST})

# 添加HTTP服务器测试可执行文件
add_executable(http_test http_server_test.cc)

# 链接库
target_link_libraries(http_test 
    mymuduo 
    pthread 
    ${MySQL_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${TINYXML2_LIBRARIES}
)


