@echo off
echo ================================================
echo   OBS Multistream Plugin Build Script
echo ================================================
echo.

REM Check if we're in the right directory
if not exist "obs-multistream.sln" (
    echo ERROR: obs-multistream.sln not found in current directory!
    echo Please run this script from the project root directory.
    pause
    exit /b 1
)

REM Set up Visual Studio environment
echo Setting up Visual Studio 2022 environment...
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64 2>nul
if errorlevel 1 (
    call "C:\Program Files\Microsoft Visual Studio\2022\Professional\VC\Auxiliary\Build\vcvarsall.bat" x64 2>nul
    if errorlevel 1 (
        call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\VC\Auxiliary\Build\vcvarsall.bat" x64 2>nul
        if errorlevel 1 (
            echo ERROR: Visual Studio 2022 not found!
            echo Please install Visual Studio 2022 with C++ development tools.
            pause
            exit /b 1
        )
    )
)

echo.
echo Building OBS Multistream Plugin...
echo Configuration: Release
echo Platform: x64
echo.

REM Build the solution
msbuild obs-multistream.sln /p:Configuration=Release /p:Platform=x64 /m /verbosity:minimal
if errorlevel 1 (
    echo.
    echo ================================================
    echo   BUILD FAILED!
    echo ================================================
    echo Please check the error messages above.
    pause
    exit /b 1
)

echo.
echo ================================================
echo   BUILD SUCCESSFUL!
echo ================================================
echo.
echo Output: bin\x64\Release\obs-multistream.dll
echo.

REM Check if OBS is installed and offer to copy the DLL
set "OBS_PLUGIN_DIR=C:\Program Files\obs-studio\obs-plugins\64bit"
if exist "%OBS_PLUGIN_DIR%" (
    echo OBS Studio detected at: C:\Program Files\obs-studio\
    echo.
    choice /C YN /M "Do you want to copy the plugin to OBS directory"
    if errorlevel 2 goto :end
    if errorlevel 1 goto :copy
) else (
    echo OBS Studio not found in default location.
    echo Please manually copy the DLL to your OBS plugins directory:
    echo "%OBS_PLUGIN_DIR%\"
    goto :end
)

:copy
echo.
echo Copying plugin to OBS directory...
copy "bin\x64\Release\obs-multistream.dll" "%OBS_PLUGIN_DIR%\" >nul
if errorlevel 1 (
    echo ERROR: Failed to copy plugin! You may need to run as administrator.
    echo Please manually copy: bin\x64\Release\obs-multistream.dll
    echo To: %OBS_PLUGIN_DIR%\
) else (
    echo Plugin copied successfully!
    echo Restart OBS Studio to load the plugin.
)

:end
echo.
echo Build complete! Press any key to exit.
pause >nul 