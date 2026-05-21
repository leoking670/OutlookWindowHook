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
- Optional hidden cold-start mode

## Command Line

```powershell
.\OlkWindowHook.exe [options]
```

Options:

| Option | Description |
| --- | --- |
| none | Start with tray icon. |
| `--no-tray` | Start without tray icon. |
| `--hotkey Ctrl+Alt+O` | Toggle the Outlook main window with a global hotkey. |
| `--start-hidden` | Start Outlook in the background after each cold start. |
| `--status` | Print whether the app is running. |
| `--exit` | Stop the running instance. |
| `--version` | Print version. |
| `--help` | Print usage. |

`--hotkey` supports `Ctrl`, `Alt`, `Shift`, and `Win` modifiers with one `A-Z`, `0-9`, or `F1-F24` key. The hotkey and hidden cold-start options are only enabled when explicitly passed on the command line.

Common combinations:

```powershell
# No tray icon, use Ctrl+Alt+O to show or hide Outlook
.\OlkWindowHook.exe --no-tray --hotkey Ctrl+Alt+O

# No tray icon, start Outlook hidden after cold start, use Ctrl+Alt+O to show it
.\OlkWindowHook.exe --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

`--no-tray` only removes the tray icon. It does not make the process a Windows GUI subsystem app. If you run it directly inside PowerShell or Windows Terminal, that terminal can remain attached while the app is running. For daily use, start it from a shortcut or a small script instead of typing the long-running command in an interactive terminal.

## Shortcuts And Startup

To create a shortcut with options:

1. Right click `OlkWindowHook.exe` and choose **Create shortcut**.
2. Right click the shortcut and choose **Properties**.
3. In **Target**, keep the executable path in quotes and add options after it:

```text
"C:\Path\To\OlkWindowHook.exe" --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

4. Use that shortcut to start the app. Put the shortcut in the Windows Startup folder if you want it to run after login.

Open the current user's Startup folder:

```powershell
explorer shell:startup
```

You can also use a PowerShell script. Create a `.ps1` file beside the executable:

```powershell
Start-Process -FilePath "$PSScriptRoot\OlkWindowHook.exe" -ArgumentList "--no-tray --start-hidden --hotkey Ctrl+Alt+O" -WindowStyle Hidden
```

Run the script from a shortcut with:

```text
powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -File "C:\Path\To\Start-OlkWindowHook.ps1"
```

For a one-line shortcut without a script, use:

```text
powershell.exe -WindowStyle Hidden -Command "Start-Process -FilePath 'C:\Path\To\OlkWindowHook.exe' -ArgumentList '--no-tray --start-hidden --hotkey Ctrl+Alt+O' -WindowStyle Hidden"
```

## How it works

1. Watches for supported top-level Outlook main windows created by `olk.exe` or `outlook.exe`
2. Installs a `WH_CALLWNDPROC` hook only on the Outlook window thread
3. Intercepts the tracked window's `WM_CLOSE` event and hides new Outlook or minimizes classic Outlook
4. Optionally registers a global hotkey to show or hide the tracked Outlook main window

## Building

Build `Release|x64` with Visual Studio 2022 or MSBuild. Releases are distributed as a zip containing `OlkWindowHook.exe`, `OlkWindowHook.dll`, `LICENSE`, and `README.md`; MSI installer builds are no longer provided.

## Licence

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
