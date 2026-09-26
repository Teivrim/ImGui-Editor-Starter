NOTICE — ImGui Editor Starter Kit
==================================

## Собственный код

© TEIVRIM 2026. Лицензия MIT, файл `LICENSE`.

## Сторонние компоненты

### Dear ImGui

- **Версия:** 1.91.4
- **Лицензия:** MIT
- **Источник:** https://github.com/ocornut/imgui
- **Как используется:** third_party/ (вендорён, офлайн)

### glad

- **Версия:** 2.0.8
- **Лицензия:** MIT
- **Источник:** https://github.com/Dav1dde/glad
- **Как используется:** third_party/glad_loader (вендорён)

### GLFW

- **Версия:** 3.4
- **Лицензия:** zlib
- **Источник:** https://www.glfw.org/license.html
- **Как используется:** third_party/glfw (вендорён)

### stb

- **Версия:** master
- **Лицензия:** public domain / MIT (двойная)
- **Источник:** https://github.com/nothings/stb
- **Как используется:** third_party/stb (вендорён)

## Что не собирается и почему

Проект **не собирается из коробки**. Причина — отсутствующий загрузчик
функций OpenGL, а не настройка сборки.

Что происходит: `src/render/GL.h` подключает `<glad/gl.h>`, а
`src/render/Renderer.cpp:23` вызывает `gladLoadGL(glfwGetProcAddress)`.
Дальше код использует 15 функций ядра OpenGL 3.x:

    glGenVertexArrays, glBindVertexArray, glCreateShader, glShaderSource,
    glCompileShader, glCreateProgram, glGenFramebuffers, glBindFramebuffer,
    glGenBuffers, glBindBuffer, glUniformMatrix4fv, glActiveTexture,
    glGenRenderbuffers, glVertexAttribPointer, glDeleteVertexArrays

Эти функции **отсутствуют в `opengl32.lib`** (в нём только GL 1.1), поэтому
линковка напрямую невозможна — нужен рантайм-загрузчик.

Состояние зависимостей:
- `third_party/glfw` — полный, GLFW 3.4, собирается.
- `third_party/imgui` — полный, 1.91.4, собирается.
- `third_party/stb` — полный, header-only, собирается.
- `third_party/glad_loader` — **неполный**: один файл
  `include/glad/gl.h` на 3 970 байт, который лишь включает
  `<GL/glcorearb.h>` и объявляет два указателя (`glClear`, `glClearColor`).
  Нет ни `src/glad.c`, ни `include/glad/glad.h`.
- Путь через CMake `FetchContent` тоже нерабочий: в теге `v2.0.8`
  репозитория `Dav1dde/glad` нет файла `src/glad.c`, конфигурация падает с
  `Cannot find source file`.

Три способа починить, от простого к правильному:

1. **Вендорить собранный GLEW.** Взять `glew32.dll` (он уже есть в
   `polygon-editor/lib/`) + заголовок `GL/glew.h` из исходников GLEW,
   собрать import-lib для MinGW через `dlltool` из .def-файла. Дальше
   `GL.h` -> `#include <GL/glew.h>`, в `Renderer.cpp` заменить
   `gladLoadGL(...)` на `glewInit()`, в CMake линковать `glew32 opengl32`.
2. **Написать загрузчик самому.** Один `.c` с массивом указателей,
   заполняемых через `glfwGetProcAddress`, плюс заголовок с `extern`-объявлениями.
   Объём — около 50 функций, но зависимостей ноль.
3. **Взять glad с правильной ветки.** В `Dav1dde/glad` готовые
   `src/glad.c` лежат в ветке `glad`, а не в теге `v2.0.8`:
   `GIT_TAG glad` вместо `GIT_TAG v2.0.8`.

До починки продукт покупать не стоит: это $19 за неработающий каркас.

## Требования к атрибуции

При распространении скомпилированного бинарника или исходников:

1. Сохраните текст MIT/zlib/BSD-разрешения, полученный вместе с компонентом (обычно лежит в корне репозитория компонента как `LICENSE`, `LICENSE.txt` или `COPYING`).
2. Для vendored-компонентов (поставляемых в исходниках) — их тексты лицензий уже лежат рядом с кодом компонента; не удаляйте их при модификации дерева.
3. Этот файл поставляйте вместе с продуктом.

## Отказ от гарантий

Продукт предоставляется «как есть» (as-is), без гарантий точности и пригодности, в соответствии с MIT. Ответственность покупателя — проверить пригодность кода до продакшена.
