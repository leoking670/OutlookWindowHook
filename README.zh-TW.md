# Outlook Window Hook

讓 Outlook 主視窗關閉後仍可在背景執行，行為類似 Teams。

支援新版 Outlook 用戶端 (`olk.exe`)，並提供 Classic Outlook (`outlook.exe`) 主視窗的基本處理。

這是由 [leoking670](https://github.com/leoking670) 維護的 fork，並由 OpenAI Codex 輔助開發。原始專案作者為 [Oliver Dalton / Palsternakka](https://github.com/Palsternakka/OutlookWindowHook)。

英文文件請見 [README.md](README.md)。

## 下載

請從 [Releases 頁面](https://github.com/leoking670/OutlookWindowHook/releases) 下載 x64 binary zip。解壓縮後，請確認 `OlkWindowHook.dll` 和 `OlkWindowHook.exe` 放在同一個資料夾。

如果 Windows 顯示缺少 `VCRUNTIME140.dll`，請先安裝 Microsoft Visual C++ Redistributable 2015-2022 x64。

## 功能

- 關閉 Outlook 主視窗後，讓 Outlook 繼續在背景執行
- 支援新版 Outlook，並提供 Classic Outlook 主視窗的基本處理
- 可選擇顯示或不顯示系統匣圖示
- 可設定全域熱鍵，用來顯示或隱藏 Outlook
- 可讓 Outlook 每次冷啟動後從背景啟動

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

## 開機啟動方式

**重要：** 系統匣選單中的 **Autostart** 只會啟動不帶參數的 `OlkWindowHook.exe`。它不會記住 `--no-tray`、`--hotkey` 或 `--start-hidden`。

如果你要設定帶參數的開機啟動，請先在系統匣圖示上按右鍵，取消勾選 **Autostart**。否則 Windows 可能會另外啟動一個無參數的基本版本，看起來就像你的參數沒有生效。

請使用下面任一方式設定。

### 方式一：帶參數的捷徑

這通常是最簡單、不需要寫程式的方法。

1. 把解壓縮後的檔案放在固定位置，例如：

```text
C:\Users\YourName\Apps\OutlookWindowHook\
```

2. 確認 `OlkWindowHook.exe` 和 `OlkWindowHook.dll` 在同一個資料夾。
3. 在 `OlkWindowHook.exe` 上按右鍵，選擇 **建立捷徑**。
4. 在新捷徑上按右鍵，選擇 **內容**。
5. 在 **目標** 欄位保留加上引號的 exe 路徑，並在後面加入參數：

```text
"C:\Path\To\OlkWindowHook.exe" --no-tray --start-hidden --hotkey Ctrl+Alt+O
```

6. 開啟目前使用者的啟動資料夾：

```powershell
explorer shell:startup
```

7. 把捷徑移到這個啟動資料夾。
8. 登出再登入，或先雙擊捷徑測試一次。

### 方式二：PowerShell 啟動腳本

如果你希望本工具先啟動，Outlook 幾秒後再啟動，請使用這種方式。

1. 在 `OlkWindowHook.exe` 同一個資料夾建立文字檔，命名為：

```text
Start-OlkWindowHook.ps1
```

2. 把下面內容貼進檔案：

```powershell
$hook = Join-Path $PSScriptRoot "OlkWindowHook.exe"
Start-Process -FilePath $hook -ArgumentList "--no-tray --start-hidden --hotkey Ctrl+Alt+O" -WindowStyle Hidden
Start-Sleep -Seconds 5
Start-Process -FilePath "outlook.exe"
```

3. `Start-Sleep` 會讓 Outlook 延後啟動，給本工具一點準備時間。如果 Outlook 還是太早打開，可以把 `5` 改大。
4. 建立一個捷徑，目標填入：

```text
powershell.exe -ExecutionPolicy Bypass -WindowStyle Hidden -File "C:\Path\To\Start-OlkWindowHook.ps1"
```

5. 用 `explorer shell:startup` 開啟啟動資料夾，把這個捷徑移進去。
6. 登出再登入測試。

### 方式三：Startup Delayer

你也可以使用 Startup Delayer 這類啟動順序管理工具：先啟動 Outlook Window Hook，再讓 Outlook 延後幾秒啟動。當 Outlook 已經由其他程式或公司政策自動啟動時，這種方式比較方便。

## Classic Outlook 注意事項

若使用 Classic Outlook：

1. 請先在 Outlook 自身設定中啟用 **最小化到系統匣**。
2. 本工具執行期間若要完整結束 Outlook，請使用 Outlook 系統匣圖示中的 **Exit**，或手動結束 `outlook.exe` 行程。

## 工作原理

1. 使用 WinEvent 通知尋找由 `olk.exe` 或 `outlook.exe` 建立的受支援 Outlook 主視窗
2. 分別追蹤新版 Outlook 和 Classic Outlook，包含各自獨立的冷啟動狀態
3. 僅針對被追蹤 Outlook 視窗所在的 thread 安裝 `WH_CALLWNDPROC` hook
4. 攔截 `WM_CLOSE`：新版 Outlook 會被隱藏，Classic Outlook 會被最小化
5. 使用 `--start-hidden` 時，新版 Outlook 會在 WebView 子視窗準備好後隱藏；Classic Outlook 則在主視窗出現時最小化
6. 若設定熱鍵，會優先控制目前前景中的 Outlook 類型，否則控制最近追蹤到的 Outlook 主視窗

## 建置

請使用 Visual Studio 2022 或 MSBuild 建置 `Release|x64`。Release 以 zip 發布，內容包含 `OlkWindowHook.exe`、`OlkWindowHook.dll`、`LICENSE` 和 README 文件。

## 授權

[GPL-3.0](https://github.com/leoking670/OutlookWindowHook?tab=GPL-3.0-1-ov-file)
