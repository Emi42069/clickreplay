cmake_minimum_required(VERSION 3.20)
project(click_replay_gui LANGUAGES CXX)

if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
    imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG docking
)
FetchContent_MakeAvailable(imgui)

add_executable(click_replay_gui WIN32
    src/main.cpp
    src/AppState.cpp
    src/AppState.h

    src/replay/ReplayEngine.cpp
    src/replay/ReplayEngine.h
    src/replay/ReplayConfig.h
    src/replay/ReplaySnapshot.h

    src/data/Dataset.cpp
    src/data/Dataset.h

    src/input/Input.cpp
    src/input/Input.h

    src/platform/Platform.cpp
    src/platform/Platform.h

    src/stats/Statistics.cpp
    src/stats/Statistics.h

    src/gui/Gui.cpp
    src/gui/Gui.h
    src/gui/Dashboard.cpp
    src/gui/Dashboard.h
    src/gui/DatasetPage.cpp
    src/gui/DatasetPage.h
    src/gui/AnalyticsPage.cpp
    src/gui/AnalyticsPage.h
    src/gui/SettingsPage.cpp
    src/gui/SettingsPage.h
    src/gui/LogsPage.cpp
    src/gui/LogsPage.h

    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_win32.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_dx11.cpp
)

target_include_directories(click_replay_gui PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
)

target_link_libraries(click_replay_gui PRIVATE d3d11 dxgi comdlg32)

if(MSVC)
    target_compile_options(click_replay_gui PRIVATE /W3 /EHsc /DNOMINMAX)
else()
    target_compile_options(click_replay_gui PRIVATE -Wall -Wextra)
endif()
