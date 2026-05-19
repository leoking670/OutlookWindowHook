# Outlook Window Hook

Keeps Outlook running in the background when the main window is closed, similar to Teams

**Currently only works with the new Outlook client** - if there's enough interest I'd be happy to implement support for older versions!

Maintained fork by [leoking670](https://github.com/leoking670). Original project by [Oliver Dalton / Palsternakka](https://github.com/Palsternakka/OutlookWindowHook).

## Download

You can find the installer or a standalone version [on the Releases page here](https://github.com/leoking670/OutlookWindowHook/releases) - please note for the standalone version `OlkWindowHook.dll` must be in the same directory as `OlkWindowHook.exe`

## Features

- Option to add to Startup (after running, right click the system tray icon and tick Autostart)
- No need for admin privileges - installs to your AppData folder

## How it works

1. Watches for top-level windows created by `olk.exe`
2. Installs a `WH_CALLWNDPROC` hook only on the Outlook window thread
3. Intercepts the tracked window's `WM_CLOSE` event and instead calls `ShowWindow(hwnd, SW_HIDE)`

## Building the installer

To build the `OlkWindowHook.Installer` project requires the [Windows SDK MSI Tools](https://developer.microsoft.com/en-us/windows/downloads/windows-sdk/). You will need to make sure that the `OlkWindowHook.Installer` project properties (namely the `PostBuildEvent`) point to your installation of `MsiInfo.exe` - this is required to set the flags on the installer to indicate admin rights aren't required

## Licence

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
