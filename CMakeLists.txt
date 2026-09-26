cmake_minimum_required(VERSION 3.20)
project(EditorCore VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

# ── Dependencies via FetchContent ──────────────────────────────────────────
include(FetchContent)

# GLFW
FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(glfw)

# GLAD - OpenGL loader
FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(glad)

# stb_image
FetchContent_Declare(stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(stb)

# Dear ImGui
FetchContent_Declare(imgui
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG v1.91.4
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(imgui)

# ── ImGui sources ──────────────────────────────────────────────────────────
set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# ── GLAD sources ───────────────────────────────────────────────────────────
set(GLAD_SRC
    ${glad_SOURCE_DIR}/src/glad.c
)

# ── Our sources ────────────────────────────────────────────────────────────
file(GLOB_RECURSE EDITOR_SRC
    src/*.cpp
)

# ── Executable ─────────────────────────────────────────────────────────────
add_executable(EditorCore
    ${EDITOR_SRC}
    ${IMGUI_SRC}
    ${GLAD_SRC}
)

target_include_directories(EditorCore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glad_SOURCE_DIR}/include
    ${stb_SOURCE_DIR}
    ${OPENGL_INCLUDE_DIRS}
)

target_link_libraries(EditorCore PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
)

if(WIN32)
    target_link_libraries(EditorCore PRIVATE gdi32 user32 shell32)
endif()
