# Outlook Window Hook

Keeps Outlook running in the background when the main window is closed, similar to Teams.

Supports the new Outlook client (`olk.exe`) and basic main-window handling for classic Outlook (`outlook.exe`).

Maintained fork by [leoking670](https://github.com/leoking670), developed with assistance from OpenAI Codex. Original project by [Oliver Dalton / Palsternakka](https://github.com/Palsternakka/OutlookWindowHook).

Traditional Chinese documentation for Taiwan users: [README.zh-TW.md](README.zh-TW.md)

## Download

Download the x64 binary zip from the [Releases page](https://github.com/leoking670/OutlookWindowHook/releases). Extract it and keep `OlkWindowHook.dll` in the same directory as `OlkWindowHook.exe`.

## What It Does

- Keeps Outlook running when the main window is closed
- Supports new Outlook and basic classic Outlook main-window handling
- Can run with or without a tray icon
- Can register a global hotkey to show or hide Outlook
- Can start Outlook in the background after each cold start
- Does not require administrator privileges

## Classic Outlook Notes

For classic Outlook, enable Outlook's own **Minimize to tray** setting first. This tool minimizes the classic Outlook main window instead of fully hiding it, so Outlook can continue to behave like a background tray app.

While this tool is running, closing the classic Outlook main window is turned into minimize-to-tray behavior. To truly exit classic Outlook, use Outlook's own **Exit** command from its tray icon or terminate the `outlook.exe` process.

## Command Line

```powershell
.\OlkWindowHook.exe [options]
```

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

Common examples:

```powershell
# No tray icon, use Ctrl+Alt+O to show or hide Outlook
.\OlkWindowHook.exe --no-tray --hotkey Ctrl+Alt+O

# No tray icon, start Outlook hidden after cold start, use Ctrl+Alt+O to show it
.\OlkWindowHook.exe --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

`--no-tray` only removes the tray icon. If you run a long-running command directly inside PowerShell or Windows Terminal, that terminal can remain attached. For daily use, start the app from a shortcut or script.

## Shortcuts And Startup

For tray mode, you can use the built-in startup option: run the app, right click the tray icon, and tick **Autostart**.

For custom options, create a Windows shortcut to `OlkWindowHook.exe`, open **Properties**, and add options after the quoted executable path:

```text
"C:\Path\To\OlkWindowHook.exe" --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

To run that shortcut after login, put it in the current user's Startup folder:

```powershell
explorer shell:startup
```

If you use `--start-hidden` at login, this tool must start before Outlook. You can use a startup order manager such as Startup Delayer, or create a `.ps1` file beside the executable:

```powershell
Start-Process -FilePath "$PSScriptRoot\OlkWindowHook.exe" -ArgumentList "--no-tray --start-hidden --hotkey Ctrl+Alt+O" -WindowStyle Hidden
Start-Sleep -Seconds 5
Start-Process -FilePath "outlook.exe"
```

## How It Works

1. Uses WinEvent notifications to find supported Outlook main windows from `olk.exe` and `outlook.exe`
2. Tracks new Outlook and classic Outlook separately, including separate cold-start state
3. Installs a `WH_CALLWNDPROC` hook only on the Outlook window thread being tracked
4. Intercepts `WM_CLOSE`: new Outlook is hidden, classic Outlook is minimized
5. For `--start-hidden`, new Outlook is hidden after its WebView child is ready; classic Outlook is minimized when its main window appears
6. If a hotkey is configured, it controls the foreground Outlook type first, then falls back to the most recently tracked Outlook window

## Building

Build `Release|x64` with Visual Studio 2022 or MSBuild. Releases are distributed as a zip containing `OlkWindowHook.exe`, `OlkWindowHook.dll`, `LICENSE`, and README files; MSI installer builds are no longer provided.

## Licence

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
