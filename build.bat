@echo off
setlocal enabledelayedexpansion

set ROOT=%~dp0
set THIRD=%ROOT%third_party
set BUILD=%ROOT%build
set OBJ=%BUILD%\obj

if not exist "%BUILD%" mkdir "%BUILD%"
if not exist "%OBJ%" mkdir "%OBJ%"
if not exist "%OBJ%\glfw" mkdir "%OBJ%\glfw"

echo === EditorCore Build (g++) ===

:: Check if deps exist
if not exist "%THIRD%\glfw\src\context.c" (
    echo Downloading GLFW...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/glfw/glfw/archive/refs/tags/3.4.zip' -OutFile '%THIRD%\glfw.zip' -UseBasicParsing; Expand-Archive -Path '%THIRD%\glfw.zip' -DestinationPath '%THIRD%' -Force; Remove-Item '%THIRD%\glfw.zip' -Force"
    for /d %%d in ("%THIRD%\glfw*") do (
        if "%%~nxd" neq "glfw" (
            if exist "%THIRD%\glfw" rmdir /s /q "%THIRD%\glfw"
            ren "%%d" "glfw"
        )
    )
)
if not exist "%THIRD%\stb\stb_image.h" (
    echo Downloading stb...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/nothings/stb/archive/refs/heads/master.zip' -OutFile '%THIRD%\stb.zip' -UseBasicParsing; Expand-Archive -Path '%THIRD%\stb.zip' -DestinationPath '%THIRD%' -Force; Remove-Item '%THIRD%\stb.zip' -Force"
    for /d %%d in ("%THIRD%\stb*") do (
        if "%%~nxd" neq "stb" (
            if exist "%THIRD%\stb" rmdir /s /q "%THIRD%\stb"
            ren "%%d" "stb"
        )
    )
)
if not exist "%THIRD%\imgui\imgui.cpp" (
    echo Downloading ImGui...
    powershell -Command "Invoke-WebRequest -Uri 'https://github.com/ocornut/imgui/archive/refs/tags/v1.91.4.zip' -OutFile '%THIRD%\imgui.zip' -UseBasicParsing; Expand-Archive -Path '%THIRD%\imgui.zip' -DestinationPath '%THIRD%' -Force; Remove-Item '%THIRD%\imgui.zip' -Force"
    for /d %%d in ("%THIRD%\imgui*") do (
        if "%%~nxd" neq "imgui" (
            if exist "%THIRD%\imgui" rmdir /s /q "%THIRD%\imgui"
            ren "%%d" "imgui"
        )
    )
)

set CFLAGS=-std=c++17 -O2 -m64 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-sign-compare -Wno-ignored-attributes -Wno-format
set DEFS=-DIMGUI_IMPL_OPENGL_LOADER_GLAD -DGLFW_INCLUDE_NONE
set INC=-I"%ROOT%src" -I"%ROOT%\third_party\glad_loader\include" -I"%THIRD%\glfw\include" -I"%THIRD%\imgui" -I"%THIRD%\imgui\backends" -I"%THIRD%\stb"
set LIBS=-lopengl32 -lgdi32 -luser32 -lshell32 -lcomdlg32 -lmingw32 -static-libgcc -static-libstdc++

:: Step 1: GLFW
echo.
echo [1/4] GLFW...
:: Incremental GLFW: only rebuild if .a doesn't exist
if not exist "%BUILD%\libglfw3.a" (
    for %%f in ("%THIRD%\glfw\src\*.c") do (
        gcc -c "%%f" -o "%OBJ%\glfw\%%~nf.o" -I"%THIRD%\glfw\include" -I"%THIRD%\glfw\src" -D_GLFW_WIN32 -O2 -m64
        if errorlevel 1 (
            echo FAIL: %%~nf
            exit /b 1
        )
    )
    dir /b "%OBJ%\glfw\*.o" > "%OBJ%\glfw\files.txt"
    cd "%OBJ%\glfw"
    ar rcs "%BUILD%\libglfw3.a" @files.txt
    cd "%ROOT%"
    del "%OBJ%\glfw\files.txt"
    echo GLFW rebuilt
) else (
    echo GLFW cached [up to date]
)

:: Step 2: Compile GL loader (C code, must use gcc not g++)
echo.
echo [2/4] Sources...
if not exist "%OBJ%\GL.o" (
    echo   src\render\GL.c
    gcc -c "%ROOT%\src\render\GL.c" -o "%OBJ%\GL.o" -I"%ROOT%\src" -I"%ROOT%\third_party\glad_loader\include" -I"%THIRD%\imgui" -DIMGUI_IMPL_OPENGL_LOADER_GLAD -DGLFW_INCLUDE_NONE -O2 -m64
    if errorlevel 1 (
        echo   FAILED: GL.c
        exit /b 1
    )
) else (
    echo   GL.c [cached]
)
set "ALL_OBJS=%OBJ%\GL.o"
set SRCS=
set SRCS=%SRCS% src\core\Logger.cpp
set SRCS=%SRCS% src\core\Window.cpp
set SRCS=%SRCS% src\core\Application.cpp
set SRCS=%SRCS% src\render\Renderer.cpp
set SRCS=%SRCS% src\render\Shader.cpp
set SRCS=%SRCS% src\render\Texture.cpp
set SRCS=%SRCS% src\render\stb_image_write.cpp
set SRCS=%SRCS% src\render\Framebuffer.cpp
set SRCS=%SRCS% src\render\VertexArray.cpp
set SRCS=%SRCS% src\render\Mesh.cpp
set SRCS=%SRCS% src\document\Layer.cpp
set SRCS=%SRCS% src\document\Composition.cpp
set SRCS=%SRCS% src\document\Document.cpp
set SRCS=%SRCS% src\document\PixelBuffer.cpp
set SRCS=%SRCS% src\undo\UndoStack.cpp
set SRCS=%SRCS% src\ui\Panel.cpp
set SRCS=%SRCS% src\ui\DockingSpace.cpp
set SRCS=%SRCS% src\ui\Style.cpp
set SRCS=%SRCS% src\effect\Effect.cpp
set SRCS=%SRCS% src\effect\EffectPipeline.cpp
set SRCS=%SRCS% src\plugin\PluginManager.cpp
set SRCS=%SRCS% src\task\TaskScheduler.cpp
set SRCS=%SRCS% src\main.cpp

set "ALL_OBJS=%ALL_OBJS%"
set NEED_LINK=0
for %%s in (%SRCS%) do (
    set "SRC=%ROOT%%%s"
    set "OBJFILE=%OBJ%\%%~ns.o"
    powershell -Command "if (Test-Path '!OBJFILE!') { if ((Get-Item '!SRC!').LastWriteTime -gt (Get-Item '!OBJFILE!').LastWriteTime) { exit 0 } else { exit 1 } } else { exit 0 }" > nul
    if not errorlevel 1 (
        echo   %%s
        g++ -c "!SRC!" -o "!OBJFILE!" %CFLAGS% %DEFS% %INC%
        if errorlevel 1 (
            echo   FAILED: %%s
            exit /b 1
        )
        set NEED_LINK=1
    )
    set "ALL_OBJS=!ALL_OBJS! !OBJFILE!"
)

:: ImGui sources
set IMGUI_SRCS=imgui.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp
for %%s in (%IMGUI_SRCS%) do (
    set "OBJFILE=%OBJ%\%%~ns.o"
    set "SRC=%THIRD%\imgui\%%s"
    powershell -Command "if (Test-Path '!OBJFILE!') { if ((Get-Item '!SRC!').LastWriteTime -gt (Get-Item '!OBJFILE!').LastWriteTime) { exit 0 } else { exit 1 } } else { exit 0 }" > nul
    if not errorlevel 1 (
        echo   %%~ns
        g++ -c "!SRC!" -o "!OBJFILE!" %CFLAGS% %DEFS% %INC%
        if errorlevel 1 (
            echo   FAILED: %%~ns
            exit /b 1
        )
        set NEED_LINK=1
    )
    set "ALL_OBJS=!ALL_OBJS! !OBJFILE!"
)
set IMGUI_BACKENDS=imgui_impl_glfw.cpp imgui_impl_opengl3.cpp
for %%s in (%IMGUI_BACKENDS%) do (
    set "OBJFILE=%OBJ%\%%~ns.o"
    set "SRC=%THIRD%\imgui\backends\%%s"
    powershell -Command "if (Test-Path '!OBJFILE!') { if ((Get-Item '!SRC!').LastWriteTime -gt (Get-Item '!OBJFILE!').LastWriteTime) { exit 0 } else { exit 1 } } else { exit 0 }" > nul
    if not errorlevel 1 (
        echo   %%~ns
        g++ -c "%THIRD%\imgui\backends\%%s" -o "!OBJFILE!" %CFLAGS% %DEFS% %INC%
        if errorlevel 1 (
            echo   FAILED: %%~ns
            exit /b 1
        )
        set NEED_LINK=1
    )
    set "ALL_OBJS=!ALL_OBJS! !OBJFILE!"
)

:: Step 3: Link
echo.
echo [3/4] Linking...
g++ -o "%BUILD%\EditorCore.exe" %ALL_OBJS% "%BUILD%\libglfw3.a" %CFLAGS% %LIBS%
if errorlevel 1 (
    echo LINK FAILED
    exit /b 1
)

echo [4/4] Success!
echo   %BUILD%\EditorCore.exe
