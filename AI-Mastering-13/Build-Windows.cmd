@echo off
setlocal
cd /d "%~dp0"
where cmake >nul 2>&1
if errorlevel 1 (
  echo Install CMake 3.22 or newer and add it to PATH.
  pause
  exit /b 1
)
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
if errorlevel 1 goto failed
cmake --build build --config Release --target AIMastering13_VST3 AIMastering13_Standalone
if errorlevel 1 goto failed
echo.
echo BUILD COMPLETE. Plugin folder:
echo %CD%\build\AIMastering13_artefacts\Release\VST3\AI Mastering 13.vst3
echo Copy the ENTIRE .vst3 folder to C:\Program Files\Common Files\VST3
pause
exit /b 0
:failed
echo Build failed. Read the first error above. See README.md for prerequisites.
pause
exit /b 1
