# Build script cho Qt project
# Usage: .\build_qt.ps1

Write-Host "Building Qt Project..." -ForegroundColor Green

$qtPaths = @(
    "D:\install\QT\6.10.1\mingw_64\bin", #Tnhat
    "D:\Qt\6.10.1\mingw_64\bin",  #Truon 
    "C:\Qt\6.10.1\mingw_64\bin"
)
$qtPath = $null
foreach ($path in $qtPaths) {
    if (Test-Path $path) {
        $qtPath = $path
        Write-Host "Found Qt at: $qtPath" -ForegroundColor Cyan
        break
    }
}

if (-not $qtPath) {
    Write-Host "`nError: Qt not found!" -ForegroundColor Red
    Write-Host "`nSearched in:" -ForegroundColor Yellow
    foreach ($path in $qtPaths) {
        Write-Host "  - $path" -ForegroundColor Gray
    }
    Write-Host "`nPlease install Qt from: https://www.qt.io/download" -ForegroundColor Yellow
    Write-Host "`nOr use console version instead:" -ForegroundColor Cyan
    Write-Host "  .\build_console.ps1" -ForegroundColor Green
    exit 1
}

# Set Qt environment
$env:PATH = "$qtPath;$env:PATH"
$mingwParent = Split-Path (Split-Path $qtPath)
$env:PATH = "$mingwParent\..\..\Tools\mingw1310_64\bin;$env:PATH"
$env:PATH = "$mingwParent\..\..\Tools\mingw1120_64\bin;$env:PATH"

# Tạo thư mục build
if (-not (Test-Path "build")) {
    New-Item -ItemType Directory -Path "build" | Out-Null
}

Set-Location build

Write-Host "Running qmake..." -ForegroundColor Cyan
& qmake ..\PBL2.pro -spec win32-g++

if ($LASTEXITCODE -ne 0) {
    Write-Host "qmake failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Write-Host "Running mingw32-make..." -ForegroundColor Cyan
& mingw32-make

if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed!" -ForegroundColor Red
    Set-Location ..
    exit 1
}

Set-Location ..

# Copy required Qt DLLs and plugins
Write-Host "`nCopying Qt runtime files..." -ForegroundColor Cyan
$releasePath = "build\release"
if (Test-Path $releasePath) {
    # Ensure previous exe is not locking the output
    $exePath = Join-Path $releasePath "GarageManagement.exe"
    if (Test-Path $exePath) {
        try {
            Stop-Process -Name "GarageManagement" -Force -ErrorAction SilentlyContinue
        } catch {}
    }
    # Core Qt DLLs needed at runtime
    $qtDlls = @(
        "Qt6Core.dll",
        "Qt6Gui.dll",
        "Qt6Widgets.dll",
        "Qt6Svg.dll"
    )
    foreach ($dll in $qtDlls) {
        $src = Join-Path $qtPath $dll
        if (Test-Path $src) {
            Copy-Item $src $releasePath -Force
            Write-Host "  Copied $dll" -ForegroundColor Green
        } else {
            Write-Host "  WARN: Missing $dll at $qtPath" -ForegroundColor Yellow
        }
    }

    # Image format plugin for SVG
    $pluginsRoot = Join-Path (Split-Path $qtPath) "plugins"
    $imageFormatsDir = Join-Path $releasePath "imageformats"
    if (-not (Test-Path $imageFormatsDir)) { New-Item -ItemType Directory -Path $imageFormatsDir | Out-Null }
    $qsvgPlugin = Join-Path (Join-Path $pluginsRoot "imageformats") "qsvg.dll"
    if (Test-Path $qsvgPlugin) {
        Copy-Item $qsvgPlugin $imageFormatsDir -Force
        Write-Host "  Copied imageformats\\qsvg.dll" -ForegroundColor Green
    }

    # Platform plugin (Windows)
    $platformsDir = Join-Path $releasePath "platforms"
    if (-not (Test-Path $platformsDir)) { New-Item -ItemType Directory -Path $platformsDir | Out-Null }
    $qwindowsPlugin = Join-Path (Join-Path $pluginsRoot "platforms") "qwindows.dll"
    if (Test-Path $qwindowsPlugin) {
        Copy-Item $qwindowsPlugin $platformsDir -Force
        Write-Host "  Copied platforms\\qwindows.dll" -ForegroundColor Green
    } else {
        Write-Host "  WARN: Missing platforms\\qwindows.dll" -ForegroundColor Yellow
    }

    # MinGW runtime DLLs (required for Qt MinGW builds)
    $mingwCandidates = @(
        ("$mingwParent\..\..\Tools\mingw1310_64\bin"),
        ("$mingwParent\..\..\Tools\mingw1120_64\bin")
    )
    $mingwBin = $mingwCandidates | Where-Object { Test-Path $_ } | Select-Object -First 1
    if ($mingwBin) {
        $mingwDlls = @(
            "libstdc++-6.dll",
            "libgcc_s_seh-1.dll",
            "libwinpthread-1.dll"
        )
        foreach ($dll in $mingwDlls) {
            $src = Join-Path $mingwBin $dll
            if (Test-Path $src) {
                Copy-Item $src $releasePath -Force
                Write-Host "  Copied $dll" -ForegroundColor Green
            } else {
                Write-Host "  WARN: Missing $dll in $mingwBin" -ForegroundColor Yellow
            }
        }
    } else {
        Write-Host "  WARN: MinGW bin not found; runtime DLLs not copied" -ForegroundColor Yellow
    }
}

Write-Host "`nBuild successful!" -ForegroundColor Green
Write-Host "Executable: build\release\GarageManagement.exe" -ForegroundColor Yellow
Write-Host "`nRun with: .\build\release\GarageManagement.exe" -ForegroundColor Cyan
