cmake_minimum_required(VERSION 3.16)
project(inCode)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Gui Widgets REQUIRED)

# Add resources
qt_add_resources(inCode_RESOURCES src/resources.qrc)

# Explicitly list source files
set(SOURCES
    src/main.cpp
    src/MainWindow.cpp
    src/SimpleSymbolIndexer.cpp
    src/CodeAnalyzer.cpp
    src/widgets/CodeEditor.cpp
    src/widgets/PHPSyntaxHighlighter.cpp
    ${inCode_RESOURCES}
)

add_executable(inCode ${SOURCES})

target_link_libraries(inCode PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets)
