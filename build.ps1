$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Third = Join-Path $Root "third_party"
$Build = Join-Path $Root "build"
$Obj = Join-Path $Build "obj"

foreach ($d in @($Third, $Build, $Obj, (Join-Path $Obj "glfw"))) {
    if (!(Test-Path $d)) { New-Item -ItemType Directory -Path $d -Force | Out-Null }
}

Write-Host "=== EditorCore Build (g++) ===" -ForegroundColor Cyan

$gpp = "g++"
$cflags = "-std=c++17 -O2 -m64 -Wall -Wextra -Wno-missing-field-initializers -Wno-unused-parameter -Wno-sign-compare -Wno-ignored-attributes -Wno-format"
$defs = "-DIMGUI_IMPL_OPENGL_LOADER_CUSTOM -DGLFW_INCLUDE_NONE"
$inc = "-I`"$Root\src`""
$libs = "-lopengl32 -lgdi32 -luser32 -lshell32 -lmingw32 -static-libgcc -static-libstdc++"

function Get-Dep($name, $url, $dir) {
    if (Test-Path $dir) { Write-Host "  $name - ok" -ForegroundColor Gray; return }
    Write-Host "  Downloading $name..."
    try {
        $zip = Join-Path $Third "$name.zip"
        Invoke-WebRequest -Uri $url -OutFile $zip -UseBasicParsing -ErrorAction Stop
        Expand-Archive -Path $zip -DestinationPath $Third -Force
        Remove-Item $zip -Force
        $extracted = Get-ChildItem -Path $Third -Directory | Where-Object { $_.Name -like "$name*" } | Select-Object -First 1
        if ($extracted) { Rename-Item -Path $extracted.FullName -NewName $name -Force }
        Write-Host "  $name - done" -ForegroundColor Green
    } catch { Write-Host "  $name - FAILED: $_" -ForegroundColor Red }
}

Get-Dep "glfw" "https://github.com/glfw/glfw/archive/refs/tags/3.4.zip" (Join-Path $Third "glfw")
Get-Dep "stb" "https://github.com/nothings/stb/archive/refs/heads/master.zip" (Join-Path $Third "stb")
Get-Dep "imgui" "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.4.zip" (Join-Path $Third "imgui")

$inc += " -I`"$(Join-Path $Third 'glfw\include')`""
$inc += " -I`"$(Join-Path $Third 'imgui')`""
$inc += " -I`"$(Join-Path $Third 'imgui\backends')`""
$stbDir = Join-Path $Third "stb"
if (Test-Path $stbDir) { $inc += " -I`"$stbDir`"" }

Write-Host "`n[1/4] GLFW..." -ForegroundColor Yellow
$glfwLib = Join-Path $Build "libglfw3.a"
if (!(Test-Path $glfwLib)) {
    $glfwSrcDir = Join-Path $Third "glfw\src"
    $glfwInc = Join-Path $Third "glfw\include"
    $glfwObjs = @()
    foreach ($src in (Get-ChildItem $glfwSrcDir -Filter "*.c").FullName) {
        $base = [System.IO.Path]::GetFileNameWithoutExtension($src)
        $obj = Join-Path $Obj "glfw\$base.o"
        $cmd = "gcc -c `"$src`" -o `"$obj`" -I`"$glfwInc`" -D_GLFW_WIN32 -D_GLFW_BUILD_DLL -O2 -m64"
        $output = Invoke-Expression $cmd 2>&1
        if ($LASTEXITCODE -ne 0) { Write-Host "  FAIL: $base"; Write-Host $output; exit 1 }
        $glfwObjs += $obj
    }
    $objList = ($glfwObjs -join " ")
    $output = Invoke-Expression "ar rcs `"$glfwLib`" $objList" 2>&1
    if ($LASTEXITCODE -ne 0) { Write-Host "  ar failed: $output"; exit 1 }
    Write-Host "  GLFW done" -ForegroundColor Green
} else { Write-Host "  GLFW cached" -ForegroundColor Gray }

Write-Host "[2/4] Sources..." -ForegroundColor Yellow

$sources = @(
    "src\core\Logger.cpp", "src\core\Window.cpp", "src\core\Application.cpp",
    "src\render\GL.c", "src\render\Renderer.cpp", "src\render\Shader.cpp",
    "src\render\Texture.cpp", "src\render\Framebuffer.cpp", "src\render\VertexArray.cpp",
    "src\render\Mesh.cpp",
    "src\document\Layer.cpp", "src\document\Composition.cpp", "src\document\Document.cpp",
    "src\document\PixelBuffer.cpp",
    "src\undo\UndoStack.cpp",
    "src\ui\Panel.cpp", "src\ui\DockingSpace.cpp", "src\ui\Style.cpp",
    "src\effect\Effect.cpp", "src\effect\EffectPipeline.cpp",
    "src\plugin\PluginManager.cpp",
    "src\task\TaskScheduler.cpp",
    "src\main.cpp"
)

$imguiSrcs = @("imgui.cpp","imgui_draw.cpp","imgui_tables.cpp","imgui_widgets.cpp")
$imguiBackends = @("imgui_impl_glfw.cpp","imgui_impl_opengl3.cpp")

$allObjs = @()

foreach ($rel in $sources) {
    $src = Join-Path $Root $rel
    $base = [System.IO.Path]::GetFileNameWithoutExtension($rel)
    $dir = [System.IO.Path]::GetDirectoryName($rel) -replace '^src[\\/]', ''
    if ($dir -ne $rel) {
        $objDir = Join-Path $Obj $dir
        if (!(Test-Path $objDir)) { New-Item -ItemType Directory -Path $objDir -Force | Out-Null }
        $obj = Join-Path $objDir "$base.o"
    } else { $obj = Join-Path $Obj "$base.o" }
    $allObjs += $obj
    if (Test-Path $obj) { continue }
    Write-Host "  $rel"
    $cmd = "$gpp -c `"$src`" -o `"$obj`" $cflags $defs $inc"
    $output = Invoke-Expression $cmd 2>&1
    if ($LASTEXITCODE -ne 0) { Write-Host "  FAILED: $rel`n$output" -ForegroundColor Red; exit 1 }
}

$imguiDir = Join-Path $Third "imgui"
foreach ($rel in $imguiSrcs) {
    $src = Join-Path $imguiDir $rel; $base = [System.IO.Path]::GetFileNameWithoutExtension($rel)
    $obj = Join-Path $Obj "$base.o"; $allObjs += $obj
    if (Test-Path $obj) { continue }
    Write-Host "  $rel"
    $cmd = "$gpp -c `"$src`" -o `"$obj`" $cflags $defs $inc"
    $output = Invoke-Expression $cmd 2>&1
    if ($LASTEXITCODE -ne 0) { Write-Host "  FAILED: $rel`n$output" -ForegroundColor Red; exit 1 }
}
foreach ($rel in $imguiBackends) {
    $src = Join-Path $imguiDir "backends\$rel"; $base = [System.IO.Path]::GetFileNameWithoutExtension($rel)
    $obj = Join-Path $Obj "$base.o"; $allObjs += $obj
    if (Test-Path $obj) { continue }
    Write-Host "  $rel"
    $cmd = "$gpp -c `"$src`" -o `"$obj`" $cflags $defs $inc"
    $output = Invoke-Expression $cmd 2>&1
    if ($LASTEXITCODE -ne 0) { Write-Host "  FAILED: $rel`n$output" -ForegroundColor Red; exit 1 }
}

Write-Host "[3/4] Linking..." -ForegroundColor Yellow
$objList = ($allObjs -join " ")
$exe = Join-Path $Build "EditorCore.exe"
$cmd = "$gpp -o `"$exe`" $objList `"$glfwLib`" $cflags $libs"
$output = Invoke-Expression $cmd 2>&1
if ($LASTEXITCODE -ne 0) { Write-Host "  LINK FAILED`n$output" -ForegroundColor Red; exit 1 }
Write-Host "[4/4] Success!" -ForegroundColor Green
Write-Host "  $exe"
