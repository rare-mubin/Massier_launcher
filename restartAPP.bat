@echo off
:: Check if MassierLauncher is running
powershell -Command "$p = Get-Process -Name 'MassierLauncher' -ErrorAction SilentlyContinue; exit $(if ($p) { 1 } else { 0 })"
if %errorlevel% equ 1 (
    echo MassierLauncher is running. Closing it...
    :: Close the process in a separate admin window and wait for it
    powershell -Command "Start-Process powershell -ArgumentList '-NoProfile -Command \"Stop-Process -Name MassierLauncher -Force; Start-Sleep -Seconds 1\"' -Verb RunAs -Wait"
    echo Process closed.
    echo.
) else (
    echo MassierLauncher is not running.
    echo.
)

echo Restarting Massier Launcher...
start "" MassierLauncher.exe
echo.
echo Massier Launcher restarted.
echo.
pause