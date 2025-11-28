add_library(SatSkyCore STATIC
    core/base.h
    utils/azel.h
    utils/azel.cpp
)


target_include_directories(SatSkyCore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(SatSkyCore PRIVATE Qt5::Core)  # 只依赖 Core，保持干净"C:\Users\peng\Desktop\azel.txt"