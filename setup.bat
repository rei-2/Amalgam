@echo off
setlocal EnableDelayedExpansion

echo ================================
echo    Amalgam Development Setup
echo ================================
echo.

:: Check for Windows 10/11
for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
echo Detected Windows version: %VERSION%

:: Check for Visual Studio 2022
echo.
echo [1/5] Checking Visual Studio 2022...
set VS2022_PATH=""
if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe" (
    set VS2022_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Enterprise\MSBuild\Current\Bin\MSBuild.exe"
    echo Found: Visual Studio 2022 Enterprise
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe" (
    set VS2022_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Professional\MSBuild\Current\Bin\MSBuild.exe"
    echo Found: Visual Studio 2022 Professional
) else if exist "%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
    set VS2022_PATH="%ProgramFiles%\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
    echo Found: Visual Studio 2022 Community
) else (
    echo ERROR: Visual Studio 2022 not found!
    echo Please install Visual Studio 2022 with C++ development tools.
    echo Download from: https://visualstudio.microsoft.com/downloads/
    pause
    exit /b 1
)

:: Check for Git
echo.
echo [2/5] Checking Git...
git --version >nul 2>&1
if errorlevel 1 (
    echo ERROR: Git not found!
    echo Please install Git from: https://git-scm.com/download/win
    pause
    exit /b 1
) else (
    echo Git is installed
)

:: Setup vcpkg
echo.
echo [3/5] Setting up vcpkg...
if not exist "vcpkg" (
    echo Cloning vcpkg...
    git clone https://github.com/Microsoft/vcpkg.git
    if errorlevel 1 (
        echo ERROR: Failed to clone vcpkg
        pause
        exit /b 1
    )
) else (
    echo vcpkg directory already exists
)

cd vcpkg

:: Bootstrap vcpkg
echo Bootstrapping vcpkg...
if not exist "vcpkg.exe" (
    call bootstrap-vcpkg.bat
    if errorlevel 1 (
        echo ERROR: Failed to bootstrap vcpkg
        pause
        exit /b 1
    )
) else (
    echo vcpkg.exe already exists
)

:: Install dependencies
echo.
echo [4/5] Installing dependencies...
echo Installing cpr (C++ Requests library)...
vcpkg.exe install cpr:x64-windows-static-md
if errorlevel 1 (
    echo ERROR: Failed to install cpr
    pause
    exit /b 1
)

echo Installing nlohmann-json...
vcpkg.exe install nlohmann-json:x64-windows-static-md
if errorlevel 1 (
    echo ERROR: Failed to install nlohmann-json
    pause
    exit /b 1
)

echo Integrating vcpkg with Visual Studio...
vcpkg.exe integrate install
if errorlevel 1 (
    echo WARNING: Failed to integrate vcpkg with Visual Studio
    echo You may need to run this as administrator
)

cd ..

:: Initialize submodules
echo.
echo [5/6] Initializing submodules...
echo Initializing AmalgamLoader and Blackbone submodules...
git submodule update --init --recursive
if errorlevel 1 (
    echo ERROR: Failed to initialize submodules
    pause
    exit /b 1
) else (
    echo Submodules initialized successfully
)

:: Restore NuGet packages
echo.
echo [6/6] Restoring NuGet packages...
where nuget >nul 2>&1
if errorlevel 1 (
    echo WARNING: NuGet not found in PATH
    echo Downloading NuGet...
    if not exist "nuget.exe" (
        powershell -Command "Invoke-WebRequest -Uri 'https://dist.nuget.org/win-x86-commandline/latest/nuget.exe' -OutFile 'nuget.exe'"
        if errorlevel 1 (
            echo ERROR: Failed to download NuGet
            pause
            exit /b 1
        )
    )
    nuget.exe restore Amalgam.sln
) else (
    nuget restore Amalgam.sln
)

echo.
echo ================================
echo        Setup Complete!
echo ================================
echo.
echo You can now:
echo 1. Open Amalgam.sln in Visual Studio 2022
echo 2. Build the project (Release/x64 recommended)
echo 3. Output will be in: output\x64\Release\
echo.
echo Available build configurations:
echo - Release
echo - ReleaseAVX2
echo - ReleaseFreetype  
echo - ReleaseFreetypeAVX2
echo.
echo Dependencies installed:
echo - cpr (C++ Requests)
echo - nlohmann-json
echo - boost (via NuGet)
echo - libolm (embedded in source)
echo - AmalgamLoader (submodule)
echo - Blackbone (nested submodule)
echo.
pause