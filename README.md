# Outlook Window Hook

[English](README.md) | [繁體中文](README.zh-TW.md)

Keeps Outlook running in the background when the main window is closed, similar to Teams.

Supports the new Outlook client (`olk.exe`) and basic main-window handling for classic Outlook (`outlook.exe`).

Maintained fork by [leoking670](https://github.com/leoking670), developed with assistance from OpenAI Codex. Original project by [Oliver Dalton / Palsternakka](https://github.com/Palsternakka/OutlookWindowHook).

## Download

Download the x64 binary zip from the [Releases page](https://github.com/leoking670/OutlookWindowHook/releases). Extract it and keep `OlkWindowHook.dll` in the same directory as `OlkWindowHook.exe`.

If Windows reports a missing `VCRUNTIME140.dll`, install the Microsoft Visual C++ Redistributable 2015-2022 x64 first.

## What It Does

- Keeps Outlook running when the main window is closed
- Supports new Outlook and basic classic Outlook main-window handling
- Can run with or without a tray icon
- Can register a global hotkey to show or hide Outlook
- Can start Outlook in the background after each cold start

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

## Startup Options

**Important:** the tray menu's **Autostart** only starts `OlkWindowHook.exe` with no command-line options. It will not remember `--no-tray`, `--hotkey`, or `--start-hidden`.

If you configure startup with custom options, first right click the tray icon and untick **Autostart**. Otherwise Windows may start a basic no-argument copy and it can look like your options are not working.

Use one of the setup paths below.

### Option 1: Shortcut With Arguments

This is usually the easiest no-code method.

1. Put the extracted release files somewhere stable, for example:

```text
C:\Users\YourName\Apps\OutlookWindowHook\
```

2. Make sure `OlkWindowHook.exe` and `OlkWindowHook.dll` are in that same folder.
3. Right click `OlkWindowHook.exe` and choose **Create shortcut**.
4. Right click the new shortcut and choose **Properties**.
5. In **Target**, keep the exe path in quotes and add your options after it:

```text
"C:\Path\To\OlkWindowHook.exe" --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

6. Open the current user's Startup folder:

```powershell
explorer shell:startup
```

7. Move the shortcut into that Startup folder.
8. Sign out and sign in again, or double click the shortcut once to test it.

### Option 2: PowerShell Startup Script

Use this when you want this tool to start first and Outlook to start a few seconds later.

1. In the same folder as `OlkWindowHook.exe`, create a text file named:

```text
Start-OlkWindowHook.ps1
```

2. Paste this into the file:

```powershell
$hook = Join-Path $PSScriptRoot "OlkWindowHook.exe"
Start-Process -FilePath $hook -ArgumentList "--no-tray --start-hidden --hotkey Ctrl+Alt+O" -WindowStyle Hidden
Start-Sleep -Seconds 5
Start-Process -FilePath "outlook.exe"
```

3. The `Start-Sleep` line gives this tool time to start before Outlook. Increase `5` if Outlook still opens too early.
4. Create a shortcut with this target:

```text
powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -File "C:\Path\To\Start-OlkWindowHook.ps1"
```

5. Open the Startup folder with `explorer shell:startup` and move that shortcut into it.
6. Sign out and sign in again to test it.

### Option 3: Startup Delayer

You can also use a startup order manager such as Startup Delayer: start Outlook Window Hook first, then delay Outlook by a few seconds. This is useful when Outlook already starts automatically through another app or policy.

## Classic Outlook Notes

For classic Outlook:

1. Enable Outlook's own **Minimize to tray** setting.
2. To fully quit Outlook while this tool is running, use Outlook's tray icon **Exit** command, or terminate `outlook.exe`.

## How It Works

1. Uses WinEvent notifications to find supported Outlook main windows from `olk.exe` and `outlook.exe`
2. Tracks new Outlook and classic Outlook separately, including separate cold-start state
3. Installs a `WH_CALLWNDPROC` hook only on the Outlook window thread being tracked
4. Intercepts `WM_CLOSE`: new Outlook is hidden, classic Outlook is minimized
5. For `--start-hidden`, new Outlook is hidden after its WebView child is ready; classic Outlook is minimized when its main window appears
6. If a hotkey is configured, it controls the foreground Outlook type first, then falls back to the most recently tracked Outlook window

## Building

Build `Release|x64` with Visual Studio 2022 or MSBuild. Releases are distributed as a zip containing `OlkWindowHook.exe`, `OlkWindowHook.dll`, `LICENSE`, and README files.

## Licence

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
