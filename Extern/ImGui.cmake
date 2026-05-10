cmake_minimum_required(VERSION 3.13.1)

include(FetchContent)

FetchContent_Declare(
    imgui
    GIT_REPOSITORY          https://github.com/ocornut/imgui.git
    GIT_TAG                 master
    GIT_SUBMODULES_RECURSE  OFF
    GIT_SHALLOW             ON
    GIT_PROGRESS            ON
)

FetchContent_MakeAvailable(imgui)

add_library(imgui OBJECT
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
)

target_include_directories(imgui
    PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        $ENV{VK_SDK_PATH}/include
)

set_target_properties(imgui PROPERTIES DEBUG_POSTFIX "_d")
