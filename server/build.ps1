# PowerShell build script for VideoTimeline Server

Write-Host "Building VideoTimeline Server..." -ForegroundColor Cyan

# Check for Qt
$qmake6 = Get-Command qmake6 -ErrorAction SilentlyContinue
$qmake = Get-Command qmake -ErrorAction SilentlyContinue

if ($qmake6) {
    $QMAKE = "qmake6"
    Write-Host "Using qmake6" -ForegroundColor Green
} elseif ($qmake) {
    $QMAKE = "qmake"
    Write-Host "Using qmake" -ForegroundColor Yellow
} else {
    Write-Host "Error: qmake not found!" -ForegroundColor Red
    Write-Host "Please add Qt to your PATH:" -ForegroundColor Yellow
    Write-Host '  $env:Path += ";C:\Qt\6.x.x\mingw_64\bin"' -ForegroundColor Cyan
    exit 1
}

# Clean previous build
if (Test-Path "Makefile") {
    Write-Host "Cleaning..." -ForegroundColor Yellow
    & mingw32-make clean 2>$null
}

# Generate Makefile
Write-Host "Generating Makefile..." -ForegroundColor Cyan
& $QMAKE server.pro

if (-not (Test-Path "Makefile")) {
    Write-Host "Error: Failed to generate Makefile!" -ForegroundColor Red
    exit 1
}

# Build
Write-Host "Compiling..." -ForegroundColor Cyan
& mingw32-make

# Check result
if (Test-Path "server.exe") {
    Write-Host "`n✓ Build successful!" -ForegroundColor Green
    Write-Host "Run with: .\server.exe" -ForegroundColor Cyan
} else {
    Write-Host "`n✗ Build failed!" -ForegroundColor Red
    exit 1
}

