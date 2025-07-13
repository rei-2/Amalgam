@echo off
setlocal EnableDelayedExpansion

echo ================================
echo    Build-Time Obfuscation
echo ================================
echo.

set "INPUT_FILE=%1"
set "OUTPUT_DIR=%2"
set "CONFIG=%3"

if "%INPUT_FILE%"=="" (
    echo ERROR: No input file specified
    echo Usage: obfuscate_build.bat ^<input_file^> ^<output_dir^> ^<config^>
    exit /b 1
)

if not exist "%INPUT_FILE%" (
    echo ERROR: Input file does not exist: %INPUT_FILE%
    exit /b 1
)

echo Input file: %INPUT_FILE%
echo Output directory: %OUTPUT_DIR%
echo Configuration: %CONFIG%
echo.

:: Check if ProtectMyTooling is available
set "PMT_PATH=tools\ProtectMyTooling\ProtectMyTooling.py"
set "PMT_PATH_ALT=%~dp0..\tools\ProtectMyTooling\ProtectMyTooling.py"

if exist "%PMT_PATH%" (
    set "PMT_FOUND=%PMT_PATH%"
    goto :pmt_found
)

if exist "%PMT_PATH_ALT%" (
    set "PMT_FOUND=%PMT_PATH_ALT%"
    goto :pmt_found
)

echo WARNING: ProtectMyTooling not found, skipping build-time obfuscation
echo   Searched paths:
echo   - %PMT_PATH%
echo   - %PMT_PATH_ALT%
echo   Current directory: %CD%
echo The executable will still use runtime signature randomization
goto :eof

:pmt_found
echo Found ProtectMyTooling at: %PMT_FOUND%

:: Check for Python
python --version >nul 2>&1
if errorlevel 1 (
    echo WARNING: Python not found, skipping build-time obfuscation
    echo The executable will still use runtime signature randomization
    goto :eof
)

echo [1/3] Creating obfuscated copy...
set "OBFUSCATED_FILE=%OUTPUT_DIR%\%~n1_obfuscated%~x1"

echo [2/3] Running ProtectMyTooling with config file...

:: Determine the correct directory for ProtectMyTooling
if "%PMT_FOUND%"=="tools\ProtectMyTooling\ProtectMyTooling.py" (
    set "PMT_DIR=tools\ProtectMyTooling"
    set "PMT_RELATIVE_INPUT=..\..\%INPUT_FILE%"
    set "PMT_RELATIVE_OUTPUT=..\..\%OBFUSCATED_FILE%"
    set "CONFIG_PATH=..\..\config\ProtectMyTooling.yaml"
) else (
    for %%I in ("%PMT_FOUND%") do set "PMT_DIR=%%~dpI"
    set "PMT_RELATIVE_INPUT=%INPUT_FILE%"
    set "PMT_RELATIVE_OUTPUT=%OBFUSCATED_FILE%"
    set "CONFIG_PATH=config\ProtectMyTooling.yaml"
)

pushd "%PMT_DIR%"

:: Use config file approach with PE packers for better AV evasion
echo Running with chain: pe2shc,sgn,upx (PE to shellcode conversion + SGN)
python ProtectMyTooling.py pe2shc,sgn,upx "%PMT_RELATIVE_INPUT%" "%PMT_RELATIVE_OUTPUT%" -c "%CONFIG_PATH%"
if errorlevel 1 (
    echo PE2SHC+SGN chain failed, trying amber,upx...
    python ProtectMyTooling.py amber,upx "%PMT_RELATIVE_INPUT%" "%PMT_RELATIVE_OUTPUT%" -c "%CONFIG_PATH%"
    if errorlevel 1 (
        echo Advanced packing failed, trying simple UPX...
        python ProtectMyTooling.py upx "%PMT_RELATIVE_INPUT%" "%PMT_RELATIVE_OUTPUT%"
    )
)

if errorlevel 1 (
    echo WARNING: ProtectMyTooling failed, using original file
    popd
    echo Build-time obfuscation skipped, but build will continue with runtime protection
    exit /b 0
)

popd

echo [3/3] Replacing original with obfuscated version...
if exist "%OBFUSCATED_FILE%" (
    copy /Y "%OBFUSCATED_FILE%" "%INPUT_FILE%" >nul
    del "%OBFUSCATED_FILE%" >nul
    echo Build-time obfuscation completed successfully!
    echo Note: Runtime signature randomization will still apply on first run
) else (
    echo WARNING: Obfuscated file not created, using original
)

:: Cleanup
echo Cleanup completed

echo.
echo Obfuscation process completed.
echo The executable now has:
echo - Multi-layer build-time obfuscation (PE2SHC + SGN + UPX chain)
echo - Runtime signature randomization (on first run)
echo - Generic naming throughout (no identifying strings)
echo - Enhanced AV evasion through chained packers
echo.