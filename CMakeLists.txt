cmake_minimum_required(VERSION 3.10)
project(SatSky LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)   # 自动处理 Qt moc
set(CMAKE_AUTORCC ON)   # 自动处理 .qrc
set(CMAKE_AUTOUIC ON)   # 自动处理 .ui

find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)  # 查找 Qt5 组件

# 添加编译定义（必须在所有add_subdirectory之前）
add_compile_definitions(NOVA_DEBUG)

# UI 部分也建议做成库
add_subdirectory(ui) # 添加 ui 子目录

# 可执行文件
add_subdirectory(app) # 添加 app 子目录

# 核心库（推荐把 src 做成静态库或接口库）
add_subdirectory(src) # 添加 src 子目录

# 测试（可选）
option(BUILD_TESTS "Build unit tests" OFF) # 选项：是否构建测试


# 如果构建测试，启用测试并添加 tests 子目录
if(BUILD_TESTS) 
    enable_testing() # 启用测试
    add_subdirectory(tests) # 添加 tests 子目录
endif()