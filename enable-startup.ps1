# Enable Massier Launcher Startup Configuration
# This script adds Massier Launcher to Windows startup

# Check if running as administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

if (-not $isAdmin) {
    Write-Host "This operation requires administrator privileges." -ForegroundColor Yellow
    Write-Host "Restarting script as administrator..." -ForegroundColor Cyan
    
    $scriptPath = $MyInvocation.MyCommand.Path
    Start-Process powershell.exe -ArgumentList "-NoProfile -ExecutionPolicy Bypass -File `"$scriptPath`"" -Verb RunAs
    exit
}

Write-Host "Adding Massier Launcher to startup..." -ForegroundColor Yellow

# Get the current directory
$installPath = $PSScriptRoot

# Enable startup
Write-Host "Enabling startup configuration..." -ForegroundColor Cyan
& "$installPath\startup.ps1" -Action enable

Write-Host "`nSetup complete! Massier Launcher will start automatically on Windows startup." -ForegroundColor Green
Write-Host "Use Alt+Space to toggle the launcher anytime." -ForegroundColor Cyan
