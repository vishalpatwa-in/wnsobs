@echo off
echo ================================================
echo   OBS Studio Development Headers Downloader
echo ================================================
echo.

REM Create development directory
if not exist "obs-studio-dev" mkdir obs-studio-dev
cd obs-studio-dev

echo Downloading OBS Studio development headers...
echo This may take a few minutes...
echo.

REM Download the latest OBS development libraries
REM Note: Replace this URL with the actual OBS development package URL
echo Unfortunately, OBS doesn't provide pre-compiled development headers.
echo You have two options:
echo.
echo Option 1: Build OBS from source (recommended for plugin development)
echo Option 2: Use the obs-plugintemplate which includes the necessary setup
echo.

echo ================================================
echo   RECOMMENDED SOLUTION: Use OBS Plugin Template
echo ================================================
echo.
echo The OBS Plugin Template provides a complete development environment:
echo 1. Git clone https://github.com/obsproject/obs-plugintemplate
echo 2. Follow the setup instructions in the template
echo 3. Use GitHub Actions for automated building
echo.

echo ================================================
echo   ALTERNATIVE: Download Pre-built Headers
echo ================================================
echo.
echo You can also download pre-built OBS libraries from:
echo - GitHub Releases of OBS Studio
echo - Look for obs-studio-*-Full-x64.zip files
echo - Extract the include and lib directories
echo.

pause
cd .. 