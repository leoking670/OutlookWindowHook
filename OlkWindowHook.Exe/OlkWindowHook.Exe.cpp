/*
*   Outlook Window Hook
*   Keeps Outlook running when main window is closed
*   Copyright (C) 2024  Oliver Dalton
*   Modifications Copyright (C) 2026  leoking670
*   Modified in 2026 by leoking670
*
*   This program is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.

*   You should have received a copy of the GNU General Public License
*   along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <windows.h>
#include <tchar.h>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <shellapi.h>
#include <stdio.h>
#include "resource.h"

#define IDC_OPEN_GITHUB     1000
#define ID_TRAY_APP_ICON    1001
#define ID_TRAY_EXIT        1002
#define ID_TRAY_ABOUT       1003
#define ID_TRAY_AUTOSTART   1004
#define WM_SYSICON          (WM_USER + 1)

constexpr wchar_t APP_NAME[] = L"Outlook Window Hook";
constexpr wchar_t GITHUB_URL[] = L"https://github.com/leoking670/OutlookWindowHook";
constexpr wchar_t TARGET_PROCESS_NAME[] = L"olk.exe";
constexpr wchar_t TARGET_WINDOW_PROP[] = L"OlkWindowHook.TargetWindow";
constexpr wchar_t CLEANUP_MESSAGE_NAME[] = L"OlkWindowHook.CleanupSubclass";
constexpr wchar_t CONTROL_MESSAGE_NAME[] = L"OlkWindowHook.Control";
constexpr wchar_t WINDOW_CLASS_NAME[] = L"OlkWindowHookClass";
constexpr wchar_t MUTEX_NAME[] = L"OlkWindowHook";
constexpr wchar_t APP_VERSION[] = L"1.1.0";

typedef BOOL(*SET_HOOK_FOR_THREAD_PROC)(DWORD);
typedef void(*REMOVE_HOOK_FOR_THREAD_PROC)(DWORD);
typedef void(*REMOVE_HOOK_PROC)();

enum class OneShotCommand {
    None,
    Status,
    Exit,
    Version,
    Help
};

struct CommandOptions {
    bool trayEnabled = true;
    OneShotCommand command = OneShotCommand::None;
    int parseExitCode = 0;
    std::wstring invalidArg;
};

HINSTANCE hInst;
NOTIFYICONDATA notifyIconData;
HMENU hPopupMenu;
HWND hwnd;
HWND hAboutDlg = NULL;
HMODULE hModule = NULL;
HWINEVENTHOOK hCreateWindowEventHook = NULL;
SET_HOOK_FOR_THREAD_PROC SetHookForThread = NULL;
REMOVE_HOOK_FOR_THREAD_PROC RemoveHookForThread = NULL;
REMOVE_HOOK_PROC RemoveHook = NULL;
std::unordered_set<DWORD> hookedThreads;
std::unordered_set<HWND> trackedWindows;
std::unordered_map<DWORD, HWND> trackedWindowByProcess;
std::unordered_map<HWND, DWORD> trackedThreadByWindow;
UINT cleanupMessage = 0;
UINT controlMessage = 0;
bool trayEnabled = true;
bool trayIconAdded = false;

void PrintLine(const wchar_t* message) {
    wprintf(L"%s\n", message);
}

void PrintHelp() {
    wprintf(L"%s %s\n", APP_NAME, APP_VERSION);
    PrintLine(L"");
    PrintLine(L"Usage:");
    PrintLine(L"  OlkWindowHook.exe              Start with tray icon");
    PrintLine(L"  OlkWindowHook.exe --no-tray    Start in background without tray icon");
    PrintLine(L"  OlkWindowHook.exe --status     Show whether the app is running");
    PrintLine(L"  OlkWindowHook.exe --exit       Stop the running instance");
    PrintLine(L"  OlkWindowHook.exe --version    Show version");
    PrintLine(L"  OlkWindowHook.exe --help       Show this help");
}

bool IsArg(const std::wstring& arg, const wchar_t* longName, const wchar_t* shortName = NULL) {
    return _wcsicmp(arg.c_str(), longName) == 0 || (shortName && _wcsicmp(arg.c_str(), shortName) == 0);
}

bool SetOneShotCommand(CommandOptions& options, OneShotCommand command) {
    if (options.command != OneShotCommand::None) {
        options.parseExitCode = 2;
        return false;
    }

    options.command = command;
    return true;
}

CommandOptions ParseCommandLine(int argc, wchar_t* argv[]) {
    CommandOptions options;

    for (int i = 1; i < argc; i++) {
        std::wstring arg = argv[i];
        if (IsArg(arg, L"--no-tray")) {
            options.trayEnabled = false;
        }
        else if (IsArg(arg, L"--status")) {
            SetOneShotCommand(options, OneShotCommand::Status);
        }
        else if (IsArg(arg, L"--exit")) {
            SetOneShotCommand(options, OneShotCommand::Exit);
        }
        else if (IsArg(arg, L"--version")) {
            SetOneShotCommand(options, OneShotCommand::Version);
        }
        else if (IsArg(arg, L"--help", L"/?")) {
            SetOneShotCommand(options, OneShotCommand::Help);
        }
        else {
            options.invalidArg = arg;
            options.parseExitCode = 2;
        }
    }

    return options;
}

HWND FindRunningWindow() {
    return FindWindowEx(HWND_MESSAGE, NULL, WINDOW_CLASS_NAME, APP_NAME);
}

int PrintStatus() {
    if (FindRunningWindow()) {
        PrintLine(L"running");
        return 0;
    }

    PrintLine(L"not running");
    return 1;
}

int RequestRunningInstanceExit() {
    HWND runningWindow = FindRunningWindow();
    if (!runningWindow) {
        PrintLine(L"not running");
        return 1;
    }

    UINT message = RegisterWindowMessage(CONTROL_MESSAGE_NAME);
    SendMessageTimeout(runningWindow, message, 0, 0, SMTO_ABORTIFHUNG, 1000, NULL);
    PrintLine(L"exit requested");
    return 0;
}

int RunOneShotCommand(const CommandOptions& options) {
    if (options.parseExitCode != 0) {
        if (!options.invalidArg.empty()) {
            fwprintf(stderr, L"Unknown option: %s\n\n", options.invalidArg.c_str());
        }
        else {
            fwprintf(stderr, L"Only one command option can be used at a time.\n\n");
        }
        PrintHelp();
        return options.parseExitCode;
    }

    switch (options.command) {
    case OneShotCommand::Status:
        return PrintStatus();
    case OneShotCommand::Exit:
        return RequestRunningInstanceExit();
    case OneShotCommand::Version:
        wprintf(L"%s %s\n", APP_NAME, APP_VERSION);
        return 0;
    case OneShotCommand::Help:
        PrintHelp();
        return 0;
    default:
        return 0;
    }
}

INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_INITDIALOG:
        hAboutDlg = hDlg;
        return (INT_PTR)TRUE;
    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL) {
            EndDialog(hDlg, LOWORD(wParam));
            hAboutDlg = NULL;
            return (INT_PTR)TRUE;
        }
        else if (LOWORD(wParam) == IDC_OPEN_GITHUB) {
            ShellExecute(NULL, L"open", GITHUB_URL, NULL, NULL, SW_SHOWNORMAL);
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

void ShowAboutDialog(HWND hwnd, HINSTANCE hInst) {
    if (hAboutDlg) {
        SetForegroundWindow(hAboutDlg);
    }
    else {
        DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hwnd, AboutDlgProc);
    }
}

void ManageStartup(bool add) {
    TCHAR szPath[MAX_PATH];
    GetModuleFileName(NULL, szPath, MAX_PATH);

    HKEY hKey;
    LPCTSTR czStartupKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCTSTR czValueName = APP_NAME;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, czStartupKey, 0, KEY_SET_VALUE, &hKey) == ERROR_SUCCESS) {
        if (add) {
            DWORD pathSize = static_cast<DWORD>((_tcslen(szPath) + 1) * sizeof(TCHAR));
            if (RegSetValueEx(hKey, czValueName, 0, REG_SZ, (LPBYTE)szPath, pathSize) == ERROR_SUCCESS) {
                MessageBox(NULL, L"Successfully added to startup!", APP_NAME, MB_OK | MB_ICONINFORMATION);
            }
            else {
                MessageBox(NULL, L"Failed to add to startup!", APP_NAME, MB_OK | MB_ICONERROR);
            }
        }
        else {
            if (RegDeleteValue(hKey, czValueName) == ERROR_SUCCESS) {
                MessageBox(NULL, L"Successfully removed from startup!", APP_NAME, MB_OK | MB_ICONINFORMATION);
            }
            else {
                MessageBox(NULL, L"Failed to remove from startup!", APP_NAME, MB_OK | MB_ICONERROR);
            }
        }
        RegCloseKey(hKey);
    }
    else {
        MessageBox(NULL, L"Failed to open registry key!", APP_NAME, MB_OK | MB_ICONERROR);
    }
}

void AddTrayIcon(HWND window) {
    if (!trayEnabled || trayIconAdded) {
        return;
    }

    notifyIconData.cbSize = sizeof(NOTIFYICONDATA);
    notifyIconData.hWnd = window;
    notifyIconData.uID = ID_TRAY_APP_ICON;
    notifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    notifyIconData.uCallbackMessage = WM_SYSICON;
    notifyIconData.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
    wcscpy_s(notifyIconData.szTip, APP_NAME);
    trayIconAdded = Shell_NotifyIcon(NIM_ADD, &notifyIconData);
}

void RemoveTrayIcon() {
    if (trayIconAdded) {
        Shell_NotifyIcon(NIM_DELETE, &notifyIconData);
        trayIconAdded = false;
    }
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_CREATE:
        AddTrayIcon(hwnd);
        break;
    case WM_SYSICON:
        if (lParam == WM_RBUTTONUP) {
            POINT curPoint;
            GetCursorPos(&curPoint);
            SetForegroundWindow(hwnd);
            TrackPopupMenu(hPopupMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, curPoint.x, curPoint.y, 0, hwnd, NULL);
            PostMessage(hwnd, WM_NULL, 0, 0);
        }
        break;
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case ID_TRAY_EXIT:
            DestroyWindow(hwnd);
            break;
        case ID_TRAY_ABOUT:
            ShowAboutDialog(hwnd, hInst);
            break;
        case ID_TRAY_AUTOSTART:
            MENUITEMINFO mii = {
              0
            };
            mii.cbSize = sizeof(MENUITEMINFO);
            mii.fMask = MIIM_STATE;
            GetMenuItemInfo(hPopupMenu, ID_TRAY_AUTOSTART, FALSE, &mii);

            if (mii.fState & MFS_CHECKED) {
                mii.fState &= ~MFS_CHECKED;
                ManageStartup(false);
            }
            else {
                mii.fState |= MFS_CHECKED;
                ManageStartup(true);
            }

            SetMenuItemInfo(hPopupMenu, ID_TRAY_AUTOSTART, FALSE, &mii);
            break;
        }
        break;
    case WM_DESTROY:
        RemoveTrayIcon();
        PostQuitMessage(0);
        break;
    default:
        if (uMsg == controlMessage) {
            DestroyWindow(hwnd);
            return 0;
        }
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

bool IsInStartup() {
    HKEY hKey;
    LPCTSTR czStartupKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Run";
    LPCTSTR czValueName = APP_NAME;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, czStartupKey, 0, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS) {
        DWORD dwType = 0;
        DWORD dwSize = 0;
        LONG lResult = RegQueryValueEx(hKey, czValueName, NULL, &dwType, NULL, &dwSize);
        RegCloseKey(hKey);

        if (lResult == ERROR_SUCCESS) {
            return true;
        }
    }

    return false;
}

std::wstring GetProcessName(DWORD processId) {
    std::wstring processName;
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, processId);
    if (hProcess) {
        wchar_t path[MAX_PATH];
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageName(hProcess, 0, path, &size)) {
            std::wstring fullPath = path;
            size_t slashPos = fullPath.find_last_of(L"\\/");
            processName = slashPos == std::wstring::npos ? fullPath : fullPath.substr(slashPos + 1);
        }
        CloseHandle(hProcess);
    }
    return processName;
}

bool IsOlkWindow(HWND window) {
    if (!IsWindow(window) || !IsWindowVisible(window) || GetAncestor(window, GA_ROOT) != window || GetWindow(window, GW_OWNER)) {
        return false;
    }

    DWORD processId = 0;
    GetWindowThreadProcessId(window, &processId);
    if (!processId) {
        return false;
    }

    return _wcsicmp(GetProcessName(processId).c_str(), TARGET_PROCESS_NAME) == 0;
}

void TrackOlkWindow(HWND window) {
    if (!SetHookForThread || !IsOlkWindow(window)) {
        return;
    }

    DWORD processId = 0;
    DWORD threadId = GetWindowThreadProcessId(window, &processId);
    if (!threadId) {
        return;
    }

    auto trackedWindow = trackedWindowByProcess.find(processId);
    if (trackedWindow != trackedWindowByProcess.end()) {
        if (IsWindow(trackedWindow->second)) {
            return;
        }
        trackedWindowByProcess.erase(trackedWindow);
    }

    if (hookedThreads.find(threadId) == hookedThreads.end()) {
        if (!SetHookForThread(threadId)) {
            DWORD dwError = GetLastError();
            TCHAR errorMessage[256];
            _stprintf_s(errorMessage, L"Failed to install Outlook window hook! Error: %d\n", dwError);
            MessageBox(NULL, errorMessage, APP_NAME, MB_ICONERROR);
            return;
        }
        hookedThreads.insert(threadId);
    }

    if (!SetProp(window, TARGET_WINDOW_PROP, reinterpret_cast<HANDLE>(1))) {
        return;
    }

    trackedWindows.insert(window);
    trackedWindowByProcess[processId] = window;
    trackedThreadByWindow[window] = threadId;
}

void RemoveTrackedWindow(HWND window) {
    trackedWindows.erase(window);

    auto trackedThread = trackedThreadByWindow.find(window);
    if (trackedThread != trackedThreadByWindow.end()) {
        hookedThreads.erase(trackedThread->second);
        if (RemoveHookForThread) {
            RemoveHookForThread(trackedThread->second);
        }
        trackedThreadByWindow.erase(trackedThread);
    }

    for (auto it = trackedWindowByProcess.begin(); it != trackedWindowByProcess.end(); ++it) {
        if (it->second == window) {
            trackedWindowByProcess.erase(it);
            break;
        }
    }
}

BOOL CALLBACK EnumWindowsProc(HWND window, LPARAM lParam) {
    TrackOlkWindow(window);
    return TRUE;
}

void CALLBACK WinEventProc(HWINEVENTHOOK hook, DWORD event, HWND window, LONG idObject, LONG idChild, DWORD eventThread, DWORD eventTime) {
    if (idObject != OBJID_WINDOW || idChild != CHILDID_SELF) {
        return;
    }

    if (event == EVENT_OBJECT_CREATE || event == EVENT_OBJECT_SHOW) {
        TrackOlkWindow(window);
    }
    else if (event == EVENT_OBJECT_DESTROY) {
        RemoveTrackedWindow(window);
    }
}

void InitializeOutlookHooks() {
    wchar_t exePath[MAX_PATH];
    GetModuleFileName(NULL, exePath, MAX_PATH);
    std::wstring exeDir = exePath;
    exeDir = exeDir.substr(0, exeDir.find_last_of(L"\\"));

    std::wstring dllPath = exeDir + L"\\OlkWindowHook.dll";
    hModule = LoadLibrary(dllPath.c_str());
    if (!hModule) {
        MessageBox(NULL, L"Failed to load OlkWindowHook.dll", APP_NAME, MB_ICONERROR);
        return;
    }

    SetHookForThread = (SET_HOOK_FOR_THREAD_PROC)GetProcAddress(hModule, "SetHookForThread");
    RemoveHookForThread = (REMOVE_HOOK_FOR_THREAD_PROC)GetProcAddress(hModule, "RemoveHookForThread");
    RemoveHook = (REMOVE_HOOK_PROC)GetProcAddress(hModule, "RemoveHook");
    if (!SetHookForThread || !RemoveHookForThread || !RemoveHook) {
        MessageBox(NULL, L"Failed to find hook functions", APP_NAME, MB_ICONERROR);
        SetHookForThread = NULL;
        RemoveHookForThread = NULL;
        RemoveHook = NULL;
        FreeLibrary(hModule);
        hModule = NULL;
        return;
    }

    cleanupMessage = RegisterWindowMessage(CLEANUP_MESSAGE_NAME);
    EnumWindows(EnumWindowsProc, 0);

    hCreateWindowEventHook = SetWinEventHook(
        EVENT_OBJECT_CREATE,
        EVENT_OBJECT_SHOW,
        NULL,
        WinEventProc,
        0,
        0,
        WINEVENT_OUTOFCONTEXT | WINEVENT_SKIPOWNPROCESS);

    if (!hCreateWindowEventHook) {
        MessageBox(NULL, L"Failed to watch for Outlook windows", APP_NAME, MB_ICONERROR);
    }
}

void CleanupOutlookHooks() {
    if (hCreateWindowEventHook) {
        UnhookWinEvent(hCreateWindowEventHook);
        hCreateWindowEventHook = NULL;
    }

    std::unordered_set<HWND> windowsToClean = trackedWindows;
    for (HWND window : windowsToClean) {
        if (IsWindow(window)) {
            if (cleanupMessage) {
                SendMessageTimeout(window, cleanupMessage, 0, 0, SMTO_ABORTIFHUNG, 100, NULL);
            }
            RemoveProp(window, TARGET_WINDOW_PROP);
        }
        RemoveTrackedWindow(window);
    }
    trackedWindowByProcess.clear();
    trackedThreadByWindow.clear();
    hookedThreads.clear();

    if (RemoveHook) {
        RemoveHook();
    }

    if (hModule) {
        FreeLibrary(hModule);
        hModule = NULL;
    }
}

void CreateTrayIconMenu() {
    hPopupMenu = CreatePopupMenu();
    AppendMenu(hPopupMenu, MF_STRING, ID_TRAY_ABOUT, L"About");

    bool inStartup = IsInStartup();
    UINT menuState = inStartup ? MF_STRING | MF_CHECKED : MF_STRING | MF_UNCHECKED;
    AppendMenu(hPopupMenu, menuState, ID_TRAY_AUTOSTART, L"Autostart");

    AppendMenu(hPopupMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hPopupMenu, MF_STRING, ID_TRAY_EXIT, L"Exit");
}

int wmain(int argc, wchar_t* argv[]) {
    CommandOptions options = ParseCommandLine(argc, argv);
    if (options.command != OneShotCommand::None || options.parseExitCode != 0) {
        return RunOneShotCommand(options);
    }

    trayEnabled = options.trayEnabled;
    FreeConsole();

    HANDLE hMutex = CreateMutex(NULL, TRUE, MUTEX_NAME);

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (!trayEnabled) {
            return 0;
        }
        else {
            MessageBox(NULL, L"Outlook Window Hook is already running", APP_NAME, MB_OK | MB_ICONEXCLAMATION);
        }

        CloseHandle(hMutex);

        return 0;
    }
    
    hInst = GetModuleHandle(NULL);
    controlMessage = RegisterWindowMessage(CONTROL_MESSAGE_NAME);

    WNDCLASSEX wcex{};
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInst;
    wcex.hIcon = LoadIcon(hInst, MAKEINTRESOURCE(IDI_ICON));
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = WINDOW_CLASS_NAME;
    wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_ICON));

    RegisterClassEx(&wcex);

    hwnd = CreateWindowEx(0, WINDOW_CLASS_NAME, APP_NAME, 0, 0, 0, 0, 0, HWND_MESSAGE, NULL, hInst, NULL);

    if (trayEnabled) {
        CreateTrayIconMenu();
    }
    InitializeOutlookHooks();

    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    CleanupOutlookHooks();

    CloseHandle(hMutex);

    return (int)msg.wParam;
}
