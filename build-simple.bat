@echo off
echo ================================================
echo   OBS Multistream Plugin - Simple Build
echo ================================================
echo.

REM Check if we're in the right directory
if not exist "obs-multistream.sln" (
    echo ERROR: obs-multistream.sln not found in current directory!
    echo Please run this script from the project root directory.
    pause
    exit /b 1
)

echo Attempting to build without full VS environment setup...
echo.

REM Try MSBuild directly
set "MSBUILD_PATH=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
if exist "%MSBUILD_PATH%" (
    echo Found MSBuild at: %MSBUILD_PATH%
    echo Building...
    "%MSBUILD_PATH%" obs-multistream.sln /p:Configuration=Release /p:Platform=x64 /v:minimal
    if errorlevel 1 (
        echo.
        echo ================================================
        echo   BUILD FAILED!
        echo ================================================
        echo This likely means C++ development tools are not installed.
        echo Please install the "Desktop development with C++" workload in Visual Studio.
        goto :instructions
    ) else (
        echo.
        echo ================================================
        echo   BUILD SUCCESSFUL!
        echo ================================================
        goto :success
    )
) else (
    echo MSBuild not found.
    goto :instructions
)

:instructions
echo.
echo To fix this issue:
echo 1. Open Visual Studio Installer
echo 2. Click "Modify" next to Visual Studio Community 2022
echo 3. Select "Desktop development with C++" workload
echo 4. Click Install
echo.
echo Alternatively, you can:
echo - Download and install "Build Tools for Visual Studio 2022"
echo - Or use the project files in Visual Studio IDE directly
goto :end

:success
echo Output: bin\x64\Release\obs-multistream.dll
echo.

REM Check if OBS is installed and offer to copy the DLL
set "OBS_PLUGIN_DIR=C:\Program Files\obs-studio\obs-plugins\64bit"
if exist "%OBS_PLUGIN_DIR%" (
    echo OBS Studio detected!
    choice /C YN /M "Copy plugin to OBS directory"
    if errorlevel 2 goto :end
    copy "bin\x64\Release\obs-multistream.dll" "%OBS_PLUGIN_DIR%\" >nul
    if errorlevel 1 (
        echo ERROR: Failed to copy plugin!
    ) else (
        echo Plugin copied successfully! Restart OBS to load it.
    )
)

:end
echo.
pause 