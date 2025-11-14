@echo off
@REM echo Unblocking PowerShell scripts...
@REM powershell -ExecutionPolicy Bypass -Command "Get-ChildItem -Path '%~dp0*.ps1' | Unblock-File"
@REM echo.

:: Check if MassierLauncher is running
powershell -Command "$p = Get-Process -Name 'MassierLauncher' -ErrorAction SilentlyContinue; exit $(if ($p) { 1 } else { 0 })"
if %errorlevel% equ 1 (
    echo MassierLauncher is running. Closing it...
    :: Close the process in a separate admin window and wait for it
    powershell -Command "Start-Process powershell -ArgumentList '-NoProfile -Command \"Stop-Process -Name MassierLauncher -Force; Start-Sleep -Seconds 1\"' -Verb RunAs -Wait"
    echo Process closed.
    echo.
)

echo Building Massier Launcher...
g++ -o MassierLauncher.exe main.cpp MassierLauncher.cpp GUI.cpp -DUNICODE -D_UNICODE -lole32 -lshell32 -lcomctl32 -luuid -lpropsys -mwindows -std=c++17
if %ERRORLEVEL% EQU 0 (
    echo Build successful!
    echo.
    start "" MassierLauncher.exe
) else (
    echo Build failed!
)
pause