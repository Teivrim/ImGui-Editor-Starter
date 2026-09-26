$Root = Split-Path -Parent $MyInvocation.MyCommand.Path
$Third = Join-Path $Root "third_party"

Write-Host "Setting up dependencies..." -ForegroundColor Cyan

$deps = @{
    "glfw" = "https://github.com/glfw/glfw/archive/refs/tags/3.4.zip"
    "imgui" = "https://github.com/ocornut/imgui/archive/refs/tags/v1.91.4.zip"
    "glad" = "https://github.com/Dav1dde/glad/archive/refs/tags/v2.0.8.zip"
    "stb" = "https://github.com/nothings/stb/archive/refs/heads/master.zip"
}

foreach ($name in $deps.Keys) {
    $path = Join-Path $Third $name
    if (Test-Path $path) {
        Write-Host "  $name already exists, skipping" -ForegroundColor Yellow
        continue
    }

    Write-Host "  Downloading $name..." -ForegroundColor Gray
    $zip = Join-Path $Third "$name.zip"
    try {
        Invoke-WebRequest -Uri $deps[$name] -OutFile $zip -UseBasicParsing -ErrorAction Stop
        Expand-Archive -Path $zip -DestinationPath $Third -Force
        Remove-Item $zip -Force

        $extracted = Get-ChildItem -Path $Third -Directory | Where-Object { $_.Name -like "$name*" } | Select-Object -First 1
        if ($extracted) {
            Rename-Item -Path $extracted.FullName -NewName $name -Force
        }
        Write-Host "  $name done" -ForegroundColor Green
    } catch {
        Write-Host "  Failed to download $name : $_" -ForegroundColor Red
    }
}

Write-Host "`nAll dependencies set up!" -ForegroundColor Cyan
Write-Host "Now run: cd $Root && cmake -B build && cmake --build build"
