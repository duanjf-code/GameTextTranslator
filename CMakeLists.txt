cmake_minimum_required(VERSION 3.16)
project(GameTextTranslator)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)

set(SOURCES
    src/main.cpp
    src/GlobalHotkey.cpp
    src/ScreenCapture.cpp
    src/SelectionOverlay.cpp
    src/Translator.cpp
    src/ResultOverlay.cpp
    src/ConfigManager.cpp
    src/PaddleOcrEngine.cpp
    src/SettingsDialog.cpp
)

set(HEADERS
    src/GlobalHotkey.h
    src/ScreenCapture.h
    src/SelectionOverlay.h
    src/Translator.h
    src/ResultOverlay.h
    src/ConfigManager.h
    src/PaddleOcrEngine.h
    src/SettingsDialog.h
)

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Widgets
    Qt6::Gui
    Qt6::Network
    d3d11
    dxgi
)

# Qt 部署
set(QT_DEPLOY_SUPPORT Qt6::DeploySupport)
if(TARGET ${QT_DEPLOY_SUPPORT})
    qt_deploy_runtime_dependencies(
        EXECUTABLE ${PROJECT_NAME}
        POST_BUILD
    )
endif()