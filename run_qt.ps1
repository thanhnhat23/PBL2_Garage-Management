Write-Host "Running Qt Application..." -ForegroundColor Cyan

$exePath = "build\release\GarageManagement.exe"
$qtBin = "D:\Install\QT\6.10.1\mingw_64\bin"

if (Test-Path $qtBin) {
    # Ensure Qt DLLs are on PATH for the spawned process
    $env:Path = "$qtBin;$env:Path"
}

if (Test-Path $exePath) {
    # Run from project root directory so app can find Data/ folder
    & $exePath
} else {
    Write-Host "Error: Executable not found at $exePath" -ForegroundColor Red
    Write-Host "Please run .\build_qt.ps1 first to build the application." -ForegroundColor Yellow
    exit 1
}
