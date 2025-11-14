# Remove Massier Launcher Startup Configuration
# This script removes Massier Launcher from Windows startup

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "This operation requires administrator privileges." -ForegroundColor Yellow
    Write-Host "Restarting script as administrator..." -ForegroundColor Cyan
    
    $scriptPath = $MyInvocation.MyCommand.Path
    Start-Process powershell.exe -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$scriptPath`"" -Verb RunAs
    exit
}

Write-Host "Removing Massier Launcher from startup..." -ForegroundColor Yellow

# Get the current directory
$installPath = $PSScriptRoot

# Disable startup
Write-Host "Disabling startup configuration..." -ForegroundColor Cyan
& "$installPath\startup.ps1" -Action disable

Write-Host "`nMassier Launcher will no longer run automatically on Windows startup." -ForegroundColor Cyan
Write-Host "`nNote: You can still launch it manually with Alt+Space hotkey." -ForegroundColor Yellow
