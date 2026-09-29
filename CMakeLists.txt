cmake_minimum_required(VERSION 3.20)
# C нужен ради реализации GLAD: glad_impl.c написан на C,
# и без объявления языка C CMake не может собрать этот TU.
project(EditorCore VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenGL REQUIRED)

# ── Зависимости ───────────────────────────────────────────────────────────
# Всё берётся из third_party, который лежит в репозитории. Сборка не
# требует сети.
#
# Раньше здесь стоял FetchContent на GLFW, Dear ImGui, stb и glad, хотя
# все четыре уже были в дереве. Противоречие было реальным: копия
# зависимости в репозитории, а сборка её игнорировала и качала заново.
# Из-за этого проект не собирался без сети, хотя всё нужное рядом.
#
# Для glad FetchContent падал outright: в теге v2.0.8 репозитория
# Dav1dde/glad нет файла src/glad.c, и конфигурация завершалась с
# «Cannot find source file». Загрузчик GLAD 2 вендорён целиком и
# header-only, поэтому компилировать в нём нечего.
set(THIRD_PARTY "${CMAKE_CURRENT_SOURCE_DIR}/third_party")

# ── GLFW: единственная зависимость, которую нужно собрать ────────────────
set(glfw_SOURCE_DIR "${THIRD_PARTY}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory("${glfw_SOURCE_DIR}" glfw-build EXCLUDE_FROM_ALL)

# stb_image и Dear ImGui — header only, их файлы перечислены ниже явно
set(stb_SOURCE_DIR "${THIRD_PARTY}/stb")
set(imgui_SOURCE_DIR "${THIRD_PARTY}/imgui")

# ── Собственные исходники ─────────────────────────────────────────────────
# Явный список вместо GLOB. При GLOB новый файл в каталоге молча
# добавляется в сборку, и через месяц никто не помнит, откуда он взялся.
set(EDITOR_SRC
    src/render/glad_impl.c   # реализация GLAD, только этот TU
    src/core/Application.cpp
    src/core/Logger.cpp
    src/core/Window.cpp
    src/document/Composition.cpp
    src/document/Document.cpp
    src/document/Layer.cpp
    src/document/PixelBuffer.cpp
    src/effect/Effect.cpp
    src/effect/EffectPipeline.cpp
    src/main.cpp
    src/plugin/PluginManager.cpp
    src/render/Framebuffer.cpp
    src/render/Mesh.cpp
    src/render/Renderer.cpp
    src/render/Shader.cpp
    src/render/Texture.cpp
    src/render/VertexArray.cpp
    src/render/stb_image_write.cpp
    src/task/TaskScheduler.cpp
    src/ui/DockingSpace.cpp
    src/ui/Panel.cpp
    src/ui/Style.cpp
    src/undo/UndoStack.cpp
)
# Ожидаемое число файлов. Если добавили новый модуль и забыли
# дописать его в список, конфигурация скажет об этом сразу, а не
# соберётся без него молча.
set(EDITOR_SRC_EXPECTED 24)

# ImGui собирается из исходников: в нём нет готовой библиотеки
set(IMGUI_SRC
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

list(LENGTH EDITOR_SRC EDITOR_SRC_ACTUAL)
if(NOT EDITOR_SRC_ACTUAL EQUAL EDITOR_SRC_EXPECTED)
    message(FATAL_ERROR
        "Число исходников не совпадает со списком: в каталоге ${EDITOR_SRC_ACTUAL}, "
        "в списке ${EDITOR_SRC_EXPECTED}. Либо добавь файл в set(EDITOR_SRC), "
        "либо исправь ожидаемое число, если это осознанно.")
endif()

add_executable(EditorCore ${EDITOR_SRC} ${IMGUI_SRC})

target_include_directories(EditorCore PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${THIRD_PARTY}          # third_party/glad/gl.h
    ${THIRD_PARTY}/stb      # stb_image.h
    ${OPENGL_INCLUDE_DIRS}
)

target_link_libraries(EditorCore PRIVATE
    glfw
    ${OPENGL_LIBRARIES}
)

if(WIN32)
    target_link_libraries(EditorCore PRIVATE gdi32 user32 shell32)
endif()

# ── Контроль: сеть не должна понадобиться ─────────────────────────────────
# Если кто-то вернёт FetchContent, проект снова станет зависимым от
# состояния чужих репозиториев. Ловим это на конфигурации, а не на
# пайплайне через полгода.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/third_party/glad/gl.h")
    message(STATUS "GLAD 2 вендорён, сборка офлайн")
else()
    message(FATAL_ERROR
        "third_party/glad/gl.h отсутствует. GLAD 2 вендорён целиком: "
        "загрузчик header-only, компилировать нечего, тянуть из сети не нужно.")
endif()

foreach(need glfw imgui stb glad)
    if(NOT EXISTS "${THIRD_PARTY}/${need}")
        message(FATAL_ERROR
            "third_party/${need} отсутствует — сборка без сети невозможна. "
            "Зависимости вендорятся в репозиторий намеренно.")
    endif()
endforeach()
