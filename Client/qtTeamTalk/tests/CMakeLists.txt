cmake_minimum_required(VERSION 3.16)
project(TeamTalkScreenShareQualityTest LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)
add_executable(screen-share-quality-test screen-share-quality-test.cpp)
target_include_directories(screen-share-quality-test PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/.."
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../Library/TeamTalk_DLL")
target_link_libraries(screen-share-quality-test PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
enable_testing()
add_test(NAME screen-share-quality COMMAND screen-share-quality-test)
