# Outlook Window Hook

讓 Outlook 主視窗關閉後仍可在背景執行，行為類似 Teams。

支援新版 Outlook 用戶端 (`olk.exe`)，並提供 Classic Outlook (`outlook.exe`) 主視窗的基本處理。

這是由 [leoking670](https://github.com/leoking670) 維護的 fork，並由 OpenAI Codex 輔助開發。原始專案作者為 [Oliver Dalton / Palsternakka](https://github.com/Palsternakka/OutlookWindowHook)。

英文文件請見 [README.md](README.md)。

## 下載

請從 [Releases 頁面](https://github.com/leoking670/OutlookWindowHook/releases) 下載 x64 binary zip。解壓縮後，請確認 `OlkWindowHook.dll` 和 `OlkWindowHook.exe` 放在同一個資料夾。

## 功能

- 關閉 Outlook 主視窗後，讓 Outlook 繼續在背景執行
- 支援新版 Outlook，並提供 Classic Outlook 主視窗的基本處理
- 可選擇顯示或不顯示系統匣圖示
- 可設定全域熱鍵，用來顯示或隱藏 Outlook
- 可讓 Outlook 每次冷啟動後從背景啟動
- 不需要系統管理員權限

## Classic Outlook 注意事項

若使用 Classic Outlook，請先在 Outlook 自身設定中啟用 **最小化到系統匣**。本工具對 Classic Outlook 採用「最小化主視窗」而不是完全隱藏，這樣 Outlook 才能維持類似背景常駐程式的行為。

本工具執行期間，關閉 Classic Outlook 主視窗的動作會被轉成最小化到系統匣。因此若要真正結束 Classic Outlook，請使用 Outlook 系統匣圖示中的 **Exit**，或手動結束 `outlook.exe` 行程。

## 命令列

```powershell
.\OlkWindowHook.exe [options]
```

| 選項 | 說明 |
| --- | --- |
| 無 | 以系統匣圖示模式啟動。 |
| `--no-tray` | 啟動時不顯示系統匣圖示。 |
| `--hotkey Ctrl+Alt+O` | 使用全域熱鍵切換 Outlook 主視窗的顯示或隱藏。 |
| `--start-hidden` | Outlook 每次冷啟動後從背景啟動。 |
| `--status` | 顯示本工具是否正在執行。 |
| `--exit` | 停止目前正在執行的本工具實例。 |
| `--version` | 顯示版本。 |
| `--help` | 顯示使用說明。 |

`--hotkey` 支援 `Ctrl`、`Alt`、`Shift`、`Win` 修飾鍵，搭配一個 `A-Z`、`0-9` 或 `F1-F24` 按鍵。熱鍵與冷啟動背景模式只有在命令列明確傳入時才會啟用。

常見範例：

```powershell
# 不顯示系統匣圖示，使用 Ctrl+Alt+O 顯示或隱藏 Outlook
.\OlkWindowHook.exe --no-tray --hotkey Ctrl+Alt+O

# 不顯示系統匣圖示，Outlook 冷啟動後從背景啟動，並用 Ctrl+Alt+O 顯示
.\OlkWindowHook.exe --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

`--no-tray` 只是不顯示系統匣圖示。如果你直接在 PowerShell 或 Windows Terminal 中執行長駐模式，該終端機可能會一直被占用。日常使用建議透過捷徑或腳本啟動。

## 捷徑與開機啟動

若使用系統匣模式，可直接使用內建開機啟動選項：執行程式後，在系統匣圖示按右鍵並勾選 **Autostart**。

若要使用自訂參數，請建立指向 `OlkWindowHook.exe` 的 Windows 捷徑，開啟 **內容**，並在加上引號的 exe 路徑後面加入參數：

```text
"C:\Path\To\OlkWindowHook.exe" --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

若希望登入後自動執行，請把捷徑放入目前使用者的啟動資料夾：

```powershell
explorer shell:startup
```

若登入後要使用 `--start-hidden`，本工具必須先於 Outlook 啟動。你可以使用 Startup Delayer 這類啟動順序管理工具，也可以在 exe 同一個資料夾建立 `.ps1` 檔案：

```powershell
Start-Process -FilePath "$PSScriptRoot\OlkWindowHook.exe" -ArgumentList "--no-tray --start-hidden --hotkey Ctrl+Alt+O" -WindowStyle Hidden
Start-Sleep -Seconds 5
Start-Process -FilePath "outlook.exe"
```

## 工作原理

1. 使用 WinEvent 通知尋找由 `olk.exe` 或 `outlook.exe` 建立的受支援 Outlook 主視窗
2. 分別追蹤新版 Outlook 和 Classic Outlook，包含各自獨立的冷啟動狀態
3. 僅針對被追蹤 Outlook 視窗所在的 thread 安裝 `WH_CALLWNDPROC` hook
4. 攔截 `WM_CLOSE`：新版 Outlook 會被隱藏，Classic Outlook 會被最小化
5. 使用 `--start-hidden` 時，新版 Outlook 會在 WebView 子視窗準備好後隱藏；Classic Outlook 則在主視窗出現時最小化
6. 若設定熱鍵，會優先控制目前前景中的 Outlook 類型，否則控制最近追蹤到的 Outlook 主視窗

## 建置

請使用 Visual Studio 2022 或 MSBuild 建置 `Release|x64`。Release 以 zip 發布，內容包含 `OlkWindowHook.exe`、`OlkWindowHook.dll`、`LICENSE` 和 README 文件；不再提供 MSI installer。

## 授權

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
