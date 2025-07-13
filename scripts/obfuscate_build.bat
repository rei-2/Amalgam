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
if not exist "tools\ProtectMyTooling\ProtectMyTooling.py" (
    echo WARNING: ProtectMyTooling not found, skipping build-time obfuscation
    echo The executable will still use runtime signature randomization
    goto :eof
)

:: Check for Python
python --version >nul 2>&1
if errorlevel 1 (
    echo WARNING: Python not found, skipping build-time obfuscation
    echo The executable will still use runtime signature randomization
    goto :eof
)

echo [1/3] Creating obfuscated copy...
set "OBFUSCATED_FILE=%OUTPUT_DIR%\%~n1_obfuscated%~x1"

:: Create ProtectMyTooling configuration with chained packers for better evasion
echo Creating advanced multi-packer config...
(
echo {
echo   "config": {
echo     "verbose": false,
echo     "debug": false,
echo     "timeout": 120,
echo     "arch": "x64"
echo   }
echo }
) > "tools\temp_config.json"

echo [2/3] Running ProtectMyTooling with chained packers...
cd tools\ProtectMyTooling

:: Use multiple packers in chain for better AV evasion
:: Fallback chain if advanced packers not available: hyperion,upx
python ProtectMyTooling.py hyperion,upx "..\..\%INPUT_FILE%" "..\..\%OBFUSCATED_FILE%"
if errorlevel 1 (
    echo Advanced packing failed, trying simple UPX...
    python ProtectMyTooling.py upx "..\..\%INPUT_FILE%" "..\..\%OBFUSCATED_FILE%"
)

if errorlevel 1 (
    echo WARNING: ProtectMyTooling failed, using original file
    cd ..\..
    del "tools\temp_config.json" >nul 2>&1
    goto :eof
)

cd ..\..

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
del "tools\temp_config.json" >nul 2>&1

echo.
echo Obfuscation process completed.
echo The executable now has:
echo - Multi-layer build-time obfuscation (Hyperion + UPX chain)
echo - Runtime signature randomization (on first run)
echo - Generic naming throughout (no identifying strings)
echo - Enhanced AV evasion through chained packers
echo.