# PowerShell build script for VideoTimeline Client
# Windows build script

Write-Host "Building VideoTimeline Client..." -ForegroundColor Cyan

# Check for Qt
$qmake6 = Get-Command qmake6 -ErrorAction SilentlyContinue
$qmake = Get-Command qmake -ErrorAction SilentlyContinue

if ($qmake6) {
    $QMAKE = "qmake6"
    Write-Host "Using qmake6" -ForegroundColor Green
} elseif ($qmake) {
    $QMAKE = "qmake"
    Write-Host "Using qmake" -ForegroundColor Yellow
    
    # Check Qt version
    $qtVersion = qmake -query QT_VERSION
    Write-Host "Qt version: $qtVersion" -ForegroundColor Yellow
} else {
    Write-Host "Error: qmake not found in PATH!" -ForegroundColor Red
    Write-Host "Please add Qt to your PATH:" -ForegroundColor Yellow
    Write-Host '  $env:Path += ";C:\Qt\6.x.x\mingw_64\bin"' -ForegroundColor Cyan
    exit 1
}

# Check if src directory exists
if (-not (Test-Path "../src")) {
    Write-Host "Error: src directory not found!" -ForegroundColor Red
    exit 1
}

# Clean previous build
if (Test-Path "../src/Makefile") {
    Write-Host "Cleaning previous build..." -ForegroundColor Yellow
    Push-Location ../src
    make clean 2>$null
    Pop-Location
}

# Generate Makefile
Write-Host "Generating Makefile..." -ForegroundColor Cyan
Push-Location ../src
& $QMAKE VideoTimeline.pro

if (-not (Test-Path "Makefile")) {
    Write-Host "Error: Failed to generate Makefile!" -ForegroundColor Red
    Pop-Location
    exit 1
}

# Build
Write-Host "Compiling..." -ForegroundColor Cyan
& make

Pop-Location

# Check for executable
if (Test-Path "../out/VideoTimeline.exe") {
    Write-Host "`n✓ Build successful!" -ForegroundColor Green
    Write-Host "Executable: out/VideoTimeline.exe" -ForegroundColor Cyan
} elseif (Test-Path "../out/VideoTimeline") {
    Write-Host "`n✓ Build successful!" -ForegroundColor Green
    Write-Host "Executable: out/VideoTimeline" -ForegroundColor Cyan
} else {
    Write-Host "`n✗ Build failed! Executable not found." -ForegroundColor Red
    exit 1
}

