# Massier Launcher

Massier Launcher is a minimal, fast Windows application launcher. It scans Start Menu shortcuts and the shell AppsFolder, provides instant search, and launches selected entries using the appropriate Windows shell APIs.

## Features

- Fast, case-insensitive search across Start Menu and AppsFolder entries
- Launch classic executables and Start Menu shortcuts (.lnk)
- Launch Microsoft Store / UWP apps via AppUserModelID using `IApplicationActivationManager`
- Launch shell items using PIDL (`SHGetIDListFromObject` + `ShellExecuteExW` with `SEE_MASK_IDLIST`) and fall back to filesystem path when available
- Simple built-in calculator: type a single binary expression (e.g. `23-45`) and the evaluated result will appear.
- Compact borderless Win32 UI with transparency and keyboard-first navigation.
- Optional PowerShell startup scripts to register a scheduled task for silent autostart.

## Build

Requirements: MinGW (g++) on Windows, and the project uses the Windows Shell COM APIs available through the platform headers.

From PowerShell (project root):

```powershell
.\build.bat
```
build.bat script will automatically Execute `MassierLauncher.exe`
The app will run silently in the background.

## How to use

- The launcher is configured to appear with `Alt+Space` (changeable in code).
- Start typing to search or Type math expressions to use the inline calculator.
- Arrow keys navigate results and Enter launches the selected entry.

## Startup Scripts

To enable autostart (runs the launcher silently at login):

1. Open PowerShell as Administrator.
2. From the project directory run:

```powershell
.\enable-startup.ps1
```
This registers a Scheduled Task that runs a small VBS wrapper using `wscript.exe` so the launcher starts without a console window.

To remove the autostart entry, run:

```powershell
.\remove-startup.ps1
```
To check startup status

```powershell
.\startup.ps1 -Action status
```

Notes:
- The scheduled task runs with highest privileges to allow the launcher to start without UAC prompts at login.
- If you prefer, you can inspect or run `startup.ps1` directly; `enable-startup.ps1` is a convenience wrapper that calls the main script.



## Troubleshooting

- If unable to find newly added apps, use the built-in rescan feature:

  Open the launcher (`Alt+Space`), type `reset`, and press `Enter`.

## Version

Current version: **v1.02**


## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

**TL;DR:** You can use, modify, distribute, and even sell this software. Just keep the copyright notice.
# massier_launcher

---