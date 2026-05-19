# Outlook Window Hook

Keeps Outlook running in the background when the main window is closed, similar to Teams

**Currently only works with the new Outlook client** - if there's enough interest I'd be happy to implement support for older versions!

Maintained fork by [leoking670](https://github.com/leoking670). Original project by [Oliver Dalton / Palsternakka](https://github.com/Palsternakka/OutlookWindowHook).

## Download

Download the x64 binary zip from the [Releases page](https://github.com/leoking670/OutlookWindowHook/releases). Extract the zip and keep `OlkWindowHook.dll` in the same directory as `OlkWindowHook.exe`.

## Features

- Option to add to Startup (after running, right click the system tray icon and tick Autostart)
- No need for admin privileges
- Optional command-line control for background/no-tray usage

## Command line

```powershell
OlkWindowHook.exe              # Start with tray icon
OlkWindowHook.exe --no-tray    # Start in background without tray icon
OlkWindowHook.exe --status     # Print whether the app is running
OlkWindowHook.exe --exit       # Stop the running instance
OlkWindowHook.exe --version    # Print version
OlkWindowHook.exe --help       # Print usage
```

## How it works

1. Watches for top-level windows created by `olk.exe`
2. Installs a `WH_CALLWNDPROC` hook only on the Outlook window thread
3. Intercepts the tracked window's `WM_CLOSE` event and instead calls `ShowWindow(hwnd, SW_HIDE)`

## Building

Build `Release|x64` with Visual Studio 2026 or MSBuild. Releases are distributed as a zip containing `OlkWindowHook.exe`, `OlkWindowHook.dll`, `LICENSE`, and `README.md`; MSI installer builds are no longer provided.

## Licence

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
