@echo off

@REM 批处理脚本：将编译生成的 secondary_bootloader.bin 复制到指定目标目录（app_name）
@REM 使用方法：.\copy.bat app_name

@REM 检查是否传入了参数
if "%~1"=="" (
    echo error: no parameter provided
    echo usage: .\copy.bat app_name
    exit /b 1
)

@REM 设置参数变量
set "app_name=%~1"

@REM 获取上一级目录
for %%I in ("%~dp0..") do set "project_dir=%%~fI\"

@REM 计算上一级目录
for %%I in ("%project_dir%..") do set "parent_dir=%%~fI\"

@REM @REM 构建目标目录路径（相对当前工程的上一级目录）
set "target_dir=%parent_dir%%app_name%"

@REM 检查目标目录是否存在
if not exist "%target_dir%" (
    echo target_dir not exist,
    exit /b 1
)

@REM 设置源文件路径（假设在工程目录的编译输出目录中，如 Objects 或 Listings）
set "source_file=%project_dir%secondary_bootloader.bin"
@REM @REM 如果 Objects 目录中没有，尝试 Listings 目录
if not exist "%source_file%" (
    echo source_file not exist,
    exit /b 1
)

@REM @REM 执行复制操作
echo Copy secondary_bootloader.bin to %target_dir%
echo src: %source_file%
echo dst: %target_dir%

copy /Y "%source_file%" "%target_dir%\"

if %errorlevel%==0 (
    echo copy success!
    echo file copied to: %target_dir%\secondary_bootloader.bin
) else (
    echo error
    exit /b 1
)

exit /b 0
