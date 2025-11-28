add_executable(SatSky
    main.cpp
)


# 添加ui目录到包含路径，使SatSky能找到mainwindow.h
target_include_directories(SatSky PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../ui)
target_include_directories(SatSky PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../src)

find_package(Qt5 REQUIRED COMPONENTS Charts)  # 查找 Qt5 Charts 组件

target_link_libraries(SatSky PRIVATE SatSkyCore) # 链接核心库
target_link_libraries(SatSky PRIVATE SatSkyUI) # 链接 UI 库
target_link_libraries(SatSky PRIVATE Qt5::Widgets) # 链接 Qt5 Widgets 库
target_link_libraries(SatSky PRIVATE Qt5::Charts) # 链接 Qt5 Charts 库

# Windows 图标、Mac 包名等
if(WIN32)
    set_target_properties(SatSky PROPERTIES OUTPUT_NAME "SatSky") # 设置输出文件名
endif()