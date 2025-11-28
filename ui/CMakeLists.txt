# 添加Qt资源文件
qt5_add_resources(SatSkyUI_RESOURCES resources/icons.qrc)

add_library(SatSkyUI STATIC
    widgets/satsky.cpp
    widgets/satsky.h
    widgets/polarplotwidget.cpp
    widgets/polarplotwidget.h
    ${SatSkyUI_RESOURCES}
)

target_include_directories(SatSkyUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(SatSkyUI PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../src)

target_link_libraries(SatSkyUI PRIVATE Qt5::Widgets) # 链接Qt5 Widgets库
target_link_libraries(SatSkyUI PRIVATE SatSkyCore) # 链接核心库，以使用AzEl等工具类