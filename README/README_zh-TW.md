# CardputerADV-DensyaNavi — PC 工具

[日本語](README.md) | [English](README_en.md) | [한국어](README_ko.md) | [繁體中文](README_zh-TW.md) | [简体中文](README_zh-CN.md) | [Español](README_es.md) | [Français](README_fr.md)

此資料夾放置用於下載車站資料等工作的 PC 端工具。

## 下載車站資料

### 需求
- Python（建議使用 Python 3.10 或更新版本）
- 網際網路連線

不需要安裝額外套件。此腳本僅使用 Python 標準函式庫。

### 執行方式

1. 從 GitHub 下載 [`pc` 分支](https://github.com/marimutukisinnmu-rgb/CardputerADV-DensyaNavi/tree/pc) 的 ZIP 檔案。
2. 解壓縮 ZIP 檔案。
3. 在包含 `download_data.py` 的資料夾中開啟終端機，並執行以下適用的指令。

**Windows**

如果可以使用 Python Launcher 的 `py` 指令：

```cmd
py download_data.py
```

或者，如果 Python 已加入 PATH：

```cmd
python download_data.py
```

命令提示字元和 PowerShell 都可以使用。

**macOS / Linux**

如果 Python 3 已加入 PATH，請執行：

```sh
python3 download_data.py
```

### 下載後的目錄結構

成功執行後，腳本所在的資料夾旁會建立 `station_data/`，並儲存以下內容：

```text
station_data/
├── out/
├── README.md
└── LICENSE.md
```

資料會從 [Seo-4d696b75/station_database](https://github.com/Seo-4d696b75/station_database) 的 `main` 分支下載。不需要安裝 Git。

### 目前腳本的功能
- 從 GitHub 下載 ZIP 檔案
- 解壓縮 `out/` 目錄
- 保存原始專案的 `README.md` 和 `LICENSE.md`

目前不會處理或轉換資料，也不會將資料寫入 microSD 卡。

### 授權與注意事項

請查看 `station_data/LICENSE.md` 和 `station_data/README.md`，了解原始資料的授權、出處及使用條款。來源儲存庫也提醒，資料的正確性與更新程度不受保證。
