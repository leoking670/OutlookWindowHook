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
#include <commctrl.h>
#include <unordered_map>
#include <unordered_set>
#include <mutex>

#pragma comment(lib, "Comctl32.lib")

constexpr wchar_t TARGET_WINDOW_PROP[] = L"OlkWindowHook.TargetWindow";
const HANDLE TARGET_ACTION_HIDE = reinterpret_cast<HANDLE>(1);
const HANDLE TARGET_ACTION_MINIMIZE = reinterpret_cast<HANDLE>(2);
constexpr wchar_t CLEANUP_MESSAGE_NAME[] = L"OlkWindowHook.CleanupSubclass";

HINSTANCE hInstance;
std::unordered_map<DWORD, HHOOK> hooksByThread;
std::unordered_set<HWND> subclassedWindows;
std::mutex mapMutex;
UINT cleanupMessage = 0;

LRESULT CALLBACK SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);

UINT GetCleanupMessage() {
    if (!cleanupMessage) {
        cleanupMessage = RegisterWindowMessage(CLEANUP_MESSAGE_NAME);
    }
    return cleanupMessage;
}

BOOL IsTargetWindow(HWND hwnd) {
    return GetProp(hwnd, TARGET_WINDOW_PROP) != NULL;
}

void HideOrMinimizeTargetWindow(HWND hwnd) {
    if (GetProp(hwnd, TARGET_WINDOW_PROP) == TARGET_ACTION_MINIMIZE) {
        ShowWindow(hwnd, SW_MINIMIZE);
    }
    else {
        ShowWindow(hwnd, SW_HIDE);
    }
}

void RemoveSubclass(HWND hwnd) {
    RemoveWindowSubclass(hwnd, SubclassProc, 1);
    RemoveProp(hwnd, TARGET_WINDOW_PROP);

    std::lock_guard<std::mutex> lock(mapMutex);
    subclassedWindows.erase(hwnd);
}

LRESULT CALLBACK SubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    UINT cleanupMsg = GetCleanupMessage();

    if (cleanupMsg && uMsg == cleanupMsg) {
        RemoveSubclass(hwnd);
        return 0;
    }

    if (uMsg == WM_CLOSE && IsTargetWindow(hwnd)) {
        HideOrMinimizeTargetWindow(hwnd);
        return 0;
    }

    if (uMsg == WM_NCDESTROY) {
        RemoveSubclass(hwnd);
    }

    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

void EnsureSubclass(HWND hwnd) {
    std::lock_guard<std::mutex> lock(mapMutex);
    if (subclassedWindows.find(hwnd) == subclassedWindows.end()) {
        if (SetWindowSubclass(hwnd, SubclassProc, 1, 0)) {
            subclassedWindows.insert(hwnd);
        }
    }
}

LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        CWPSTRUCT* pCwp = (CWPSTRUCT*)lParam;

        if (GetCleanupMessage() && pCwp->message == cleanupMessage && IsTargetWindow(pCwp->hwnd)) {
            RemoveSubclass(pCwp->hwnd);
        }
        else if (pCwp->message == WM_CLOSE && IsTargetWindow(pCwp->hwnd)) {
            EnsureSubclass(pCwp->hwnd);
            HideOrMinimizeTargetWindow(pCwp->hwnd);
            return 0;
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

extern "C"
__declspec(dllexport) BOOL SetHookForThread(DWORD threadId) {
    std::lock_guard<std::mutex> lock(mapMutex);
    if (hooksByThread.find(threadId) != hooksByThread.end()) {
        return TRUE;
    }

    HHOOK hook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, hInstance, threadId);
    if (!hook) {
        return FALSE;
    }

    hooksByThread[threadId] = hook;
    return TRUE;
}

extern "C"
__declspec(dllexport) void RemoveHookForThread(DWORD threadId) {
    std::lock_guard<std::mutex> lock(mapMutex);
    auto hook = hooksByThread.find(threadId);
    if (hook != hooksByThread.end()) {
        UnhookWindowsHookEx(hook->second);
        hooksByThread.erase(hook);
    }
}

extern "C"
__declspec(dllexport) void RemoveHook() {
    std::lock_guard<std::mutex> lock(mapMutex);
    for (auto& hook : hooksByThread) {
        UnhookWindowsHookEx(hook.second);
    }
    hooksByThread.clear();
}

BOOL APIENTRY DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        hInstance = hinstDLL;
        DisableThreadLibraryCalls(hinstDLL);
        InitCommonControls();
        break;
    }
    return TRUE;
}
