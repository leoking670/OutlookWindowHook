# Outlook Window Hook

讓 Outlook 主視窗關閉後仍可在背景執行，行為類似 Teams。

支援新版 Outlook 用戶端 (`olk.exe`)，並提供 Classic Outlook (`outlook.exe`) 主視窗的基本處理。

這是由 [leoking670](https://github.com/leoking670) 維護的 fork，並由 OpenAI Codex 輔助開發。原始專案作者為 [Oliver Dalton / Palsternakka](https://github.com/Palsternakka/OutlookWindowHook)。

英文文件請見 [README.md](README.md)。

## 下載

請從 [Releases 頁面](https://github.com/leoking670/OutlookWindowHook/releases) 下載 x64 binary zip。解壓縮後，請確認 `OlkWindowHook.dll` 和 `OlkWindowHook.exe` 放在同一個資料夾。

## 功能

- 可加入開機啟動：程式執行後，在系統匣圖示上按右鍵並勾選 Autostart
- 不需要系統管理員權限
- 支援命令列控制，方便背景執行或無系統匣圖示使用
- 可設定全域熱鍵，用來顯示或隱藏 Outlook 主視窗
- 可選擇冷啟動時讓 Outlook 從背景啟動

## Classic Outlook 注意事項

若使用 Classic Outlook，請先在 Outlook 自身設定中啟用 **最小化到系統匣**。本工具對 Classic Outlook 採用「最小化主視窗」而不是完全隱藏，這樣 Outlook 才能維持類似背景常駐程式的行為。

本工具執行期間，關閉 Classic Outlook 主視窗的動作會被攔截並轉為最小化到系統匣。因此若要真正結束 Classic Outlook，請使用 Outlook 系統匣圖示中的 **Exit**，或手動結束 `outlook.exe` 行程。

## 命令列

```powershell
.\OlkWindowHook.exe [options]
```

選項：

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

常見組合：

```powershell
# 不顯示系統匣圖示，使用 Ctrl+Alt+O 顯示或隱藏 Outlook
.\OlkWindowHook.exe --no-tray --hotkey Ctrl+Alt+O

# 不顯示系統匣圖示，Outlook 冷啟動後從背景啟動，並用 Ctrl+Alt+O 顯示
.\OlkWindowHook.exe --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

`--no-tray` 只是不顯示系統匣圖示，並不會把程式變成真正的 Windows GUI subsystem app。如果你直接在 PowerShell 或 Windows Terminal 中執行長駐模式，該終端機可能會一直被占用。日常使用建議透過捷徑或簡短腳本啟動，不要在互動式終端機中直接輸入長駐命令。

## 捷徑與開機啟動

建立帶參數的捷徑：

1. 在 `OlkWindowHook.exe` 上按右鍵，選擇 **建立捷徑**。
2. 在捷徑上按右鍵，選擇 **內容**。
3. 在 **目標** 欄位中保留 exe 路徑的引號，並在後面加上參數：

```text
"C:\Path\To\OlkWindowHook.exe" --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

4. 使用該捷徑啟動工具。如果希望登入 Windows 後自動執行，可將捷徑放入目前使用者的啟動資料夾。

開啟目前使用者的啟動資料夾：

```powershell
explorer shell:startup
```

也可以使用 PowerShell 腳本。請在 exe 同一個資料夾建立 `.ps1` 檔案：

```powershell
Start-Process -FilePath "$PSScriptRoot\OlkWindowHook.exe" -ArgumentList "--no-tray --start-hidden --hotkey Ctrl+Alt+O" -WindowStyle Hidden
```

從捷徑執行該腳本：

```text
powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -File "C:\Path\To\Start-OlkWindowHook.ps1"
```

若不想建立腳本，也可以使用單行捷徑：

```text
powershell.exe -WindowStyle Hidden -Command "Start-Process -FilePath 'C:\Path\To\OlkWindowHook.exe' -ArgumentList '--no-tray --start-hidden --hotkey Ctrl+Alt+O' -WindowStyle Hidden"
```

## 工作原理

1. 監看由 `olk.exe` 或 `outlook.exe` 建立的受支援頂層 Outlook 主視窗
2. 僅針對 Outlook 視窗所在的 thread 安裝 `WH_CALLWNDPROC` hook
3. 攔截被追蹤視窗的 `WM_CLOSE` 事件，對新版 Outlook 執行隱藏，對 Classic Outlook 執行最小化
4. 可選擇註冊全域熱鍵，用來顯示或隱藏被追蹤的 Outlook 主視窗

## 建置

請使用 Visual Studio 2022 或 MSBuild 建置 `Release|x64`。Release 以 zip 發布，內容包含 `OlkWindowHook.exe`、`OlkWindowHook.dll`、`LICENSE` 和 README 文件；不再提供 MSI installer。

## 授權

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
