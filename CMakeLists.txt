cmake_minimum_required(VERSION 3.16)
project(NodeCodeStudio VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets)

include(FetchContent)
FetchContent_Declare(json
    URL https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz
)
FetchContent_MakeAvailable(json)

set(SOURCES

    # Core
    src/core/Node.h
    src/core/Node.cpp
    src/core/NodePort.h
    src/core/NodeLink.h
    src/core/NodeDef.h
    src/core/NodeGraph.h
    src/core/NodeGraph.cpp
    src/core/TypeSystem.h
    src/core/CodeGenerator.h
    src/core/CodeGenerator.cpp
    src/core/Serialization.h
    src/core/Serialization.cpp
    src/core/Project.h
    src/core/Project.cpp

    # GUI - Main
    src/gui/MainWindow.h
    src/gui/MainWindow.cpp
    src/gui/NodeGraphView.h
    src/gui/StyleSheet.h
    src/gui/StyleSheet.cpp

    # GUI - Items
    src/gui/items/NodePortGraphicsItem.h
    src/gui/items/NodePortGraphicsItem.cpp
    src/gui/items/NodeGraphicsItem.h
    src/gui/items/NodeGraphicsItem.cpp
    src/gui/items/NodeLinkGraphicsItem.h
    src/gui/items/NodeLinkGraphicsItem.cpp
    src/gui/items/NodeCommentItem.h
    src/gui/items/NodeCommentItem.cpp

    # GUI - Scene
    src/gui/scene/NodeGraphScene.h
    src/gui/scene/NodeGraphScene.cpp

    # GUI - Widgets
    src/gui/widgets/SearchableMenu.h
    src/gui/widgets/SearchableMenu.cpp
    src/gui/widgets/SyntaxHighlighter.h
    src/gui/widgets/SyntaxHighlighter.cpp

    # GUI - Panels
    src/gui/panels/CodePreviewPanel.h
    src/gui/panels/CodePreviewPanel.cpp
    src/gui/panels/ConsolePanel.h
    src/gui/panels/ConsolePanel.cpp
    src/gui/panels/NodeLibraryPanel.h
    src/gui/panels/NodeLibraryPanel.cpp
    src/gui/panels/VariablesPanel.h
    src/gui/panels/VariablesPanel.cpp

    # Compiler
    src/compiler/CompilerBackend.h
    src/compiler/CompilerBackend.cpp

    # Plugin
    src/plugin/PluginManager.h
    src/plugin/PluginManager.cpp

    resources.qrc
)

add_executable(NodeCode src/main.cpp ${SOURCES})

target_include_directories(NodeCode PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(NodeCode PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    nlohmann_json::nlohmann_json
)

include(CTest)
if(BUILD_TESTING)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    enable_testing()

    add_executable(NodeCodeTests 
        tests/test_node.cpp 
        ${SOURCES}
    )
    
    target_include_directories(NodeCodeTests PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
    )
    
    target_link_libraries(NodeCodeTests PRIVATE
        Qt6::Core
        Qt6::Gui
        Qt6::Widgets
        nlohmann_json::nlohmann_json
        gtest_main
    )

    include(GoogleTest)
    gtest_discover_tests(NodeCodeTests)
endif()
