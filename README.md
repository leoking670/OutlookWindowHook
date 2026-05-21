# Outlook Window Hook

Keeps Outlook running in the background when the main window is closed, similar to Teams.

Supports the new Outlook client (`olk.exe`) and basic main-window handling for classic Outlook (`outlook.exe`).

Maintained fork by [leoking670](https://github.com/leoking670). Original project by [Oliver Dalton / Palsternakka](https://github.com/Palsternakka/OutlookWindowHook).

## Download

Download the x64 binary zip from the [Releases page](https://github.com/leoking670/OutlookWindowHook/releases). Extract the zip and keep `OlkWindowHook.dll` in the same directory as `OlkWindowHook.exe`.

## Features

- Option to add to Startup (after running, right click the system tray icon and tick Autostart)
- No need for admin privileges
- Optional command-line control for background/no-tray usage
- Optional global hotkey to show or hide the Outlook main window
- Optional first-open background mode

## Command line

```powershell
OlkWindowHook.exe              # Start with tray icon
OlkWindowHook.exe --no-tray    # Start in background without tray icon
OlkWindowHook.exe --hotkey Ctrl+Alt+O
                              # Toggle the Outlook main window with a global hotkey
OlkWindowHook.exe --hide-on-first-open
                              # Hide the first Outlook main window opened after each cold start
OlkWindowHook.exe --status     # Print whether the app is running
OlkWindowHook.exe --exit       # Stop the running instance
OlkWindowHook.exe --version    # Print version
OlkWindowHook.exe --help       # Print usage
```

`--hotkey` supports `Ctrl`, `Alt`, `Shift`, and `Win` modifiers with one `A-Z`, `0-9`, or `F1-F24` key. The hotkey and cold-start hiding options are only enabled when explicitly passed on the command line.

## How it works

1. Watches for supported top-level Outlook main windows created by `olk.exe` or `outlook.exe`
2. Installs a `WH_CALLWNDPROC` hook only on the Outlook window thread
3. Intercepts the tracked window's `WM_CLOSE` event and hides new Outlook or minimizes classic Outlook
4. Optionally registers a global hotkey to show or hide the tracked Outlook main window

## Building

Build `Release|x64` with Visual Studio 2022 or MSBuild. Releases are distributed as a zip containing `OlkWindowHook.exe`, `OlkWindowHook.dll`, `LICENSE`, and `README.md`; MSI installer builds are no longer provided.

## Licence

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
