@echo off
setlocal

REM Anchor to this script's directory so relative paths work no matter how it's invoked.
cd /d "%~dp0"

echo ========================================
echo  Ratosaurus VST3 Build Script
echo ========================================
echo  Working dir: %CD%
echo.

REM --- Locate CMake ---
set "CMAKE_EXE="
where cmake >nul 2>&1
if %errorlevel%==0 (
    set "CMAKE_EXE=cmake"
) else if exist "C:\Program Files\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=C:\Program Files\CMake\bin\cmake.exe"
) else if exist "%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe" (
    set "CMAKE_EXE=%LOCALAPPDATA%\Programs\CMake\bin\cmake.exe"
)
if "%CMAKE_EXE%"=="" (
    echo ERROR: CMake not found. Install CMake 3.22+ and add to PATH.
    exit /b 1
)
echo CMake: %CMAKE_EXE%

REM --- Pick the highest installed Visual Studio generator ---
set "VS_GEN=Visual Studio 17 2022"
for %%E in (Community Professional Enterprise BuildTools) do (
    if exist "%ProgramFiles%\Microsoft Visual Studio\17\%%E" set "VS_GEN=Visual Studio 17 2022"
)
for %%E in (Community Professional Enterprise BuildTools) do (
    if exist "%ProgramFiles%\Microsoft Visual Studio\18\%%E" set "VS_GEN=Visual Studio 18 2026"
)
echo Generator: %VS_GEN%
echo.

if not exist build mkdir build
cd build

echo [1/2] Configuring (first run clones JUCE 8.0.13 ~ 1-5 min)...
"%CMAKE_EXE%" .. -G "%VS_GEN%" -A x64
if %errorlevel% neq 0 (
    echo.
    echo Configuration failed. Make sure Visual Studio with C++ workload is installed.
    exit /b 1
)

echo.
echo [2/2] Building Release...
"%CMAKE_EXE%" --build . --config Release --parallel
if %errorlevel% neq 0 (
    echo Build failed.
    exit /b 1
)

echo.
echo ========================================
echo  Build successful!
echo.
echo  VST3 location:
echo  build\Ratosaurus_artefacts\Release\VST3\Ratosaurus.vst3
echo.
echo  To install, copy to:
echo  C:\Program Files\Common Files\VST3\
echo ========================================

endlocal
