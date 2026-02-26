@echo off
setlocal

rem =========================================================
rem Configuration Section - Modify only these variables
rem This script should be placed in %BASE_PATH%\%PLATFORM_DIR%
rem =========================================================

rem Project root directory (absolute path)
set BASE_PATH=D:\Program Files\ING_SDK\boot_2\Secondary_Bootloader\ING20xx

rem Platform SDK directory name (contains scripts and generated)
set PLATFORM_DIR=2_platform_companion

rem ROM SDK directory name (target root directory)
set ROM_DIR=3_app

rem gen_files tool path (absolute path, usually needs modification per PC)
set GEN_FILES_TOOL_DIR=D:\Program Files\ING_SDK\tools

rem =========================================================
rem Relative Path Configuration (usually no modification needed)
rem =========================================================

rem Step 1: Python script configuration
set PYTHON_SCRIPT_DIR=scripts
set PYTHON_SCRIPT_NAME=make_bundle.py
set GEN_FILES_TOOL_NAME=gen_files.nim
set GEN_FILES_TOOL_CMD=gen_files

rem Step 2: File copy configuration
set SOURCE_DIR=generated
set DEST_SUBDIR=sdk\bundles\rom\ING20xx

rem =========================================================
rem Execution Logic (DO NOT MODIFY BELOW)
rem =========================================================

rem Build full paths
set SCRIPT_PATH=%BASE_PATH%\%PLATFORM_DIR%
set SOURCE_PATH=%SCRIPT_PATH%\%SOURCE_DIR%
set DEST_PATH=%BASE_PATH%\%ROM_DIR%\%DEST_SUBDIR%
set GEN_FILES_TARGET=%BASE_PATH%\%ROM_DIR%\sdk\bundles

rem Change to script directory
cd /d "%SCRIPT_PATH%"
if errorlevel 1 (
    echo Error: Cannot change to script directory: %SCRIPT_PATH%
    exit /b 1
)

echo Current working directory: %CD%
echo.

rem Step 1: Execute Python script
echo [Step 1] Executing Python script: %PYTHON_SCRIPT_NAME%
pushd "%PYTHON_SCRIPT_DIR%"
python "%PYTHON_SCRIPT_NAME%"
if errorlevel 1 (
    popd
    echo Error: Python script execution failed
    exit /b 1
)
popd
echo [Step 1] Python script execution completed
echo.

rem Step 2: Copy files
echo [Step 2] Copying files to destination directory...
if not exist "%SOURCE_PATH%" (
    echo Error: Source directory not found: %SOURCE_PATH%
    exit /b 1
)

if not exist "%DEST_PATH%" (
    echo Creating destination directory: %DEST_PATH%
    mkdir "%DEST_PATH%"
)

xcopy "%SOURCE_PATH%\*" "%DEST_PATH%" /E /Y /I
if errorlevel 1 (
    echo Error: File copy operation failed
    exit /b 1
)
echo [Step 2] File copy operation completed
echo.


rem Change to gen_files tool directory
cd /d "%GEN_FILES_TOOL_DIR%"
if errorlevel 1 (
    echo Error: Cannot change to gen_files tool directory: %GEN_FILES_TOOL_DIR%
    exit /b 1
)

echo Current working directory: %CD%
echo.

rem Step 3: Execute gen_files tool
echo [Step 3] Executing gen_files tool...
if not exist "%GEN_FILES_TOOL_NAME%" (
    echo Error: gen_files tool not found: %GEN_FILES_TOOL_NAME%
    exit /b 1
)

"%GEN_FILES_TOOL_CMD%" "%GEN_FILES_TARGET%"
if errorlevel 1 (
    echo Error: gen_files execution failed
    exit /b 1
)
echo [Step 3] gen_files execution completed
echo.

echo All steps completed successfully!
exit /b 0