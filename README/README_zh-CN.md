# CardputerADV-DensyaNavi — PC 工具

[日本語](README.md) | [English](README_en.md) | [한국어](README_ko.md) | [繁體中文](README_zh-TW.md) | [简体中文](README_zh-CN.md) | [Español](README_es.md) | [Français](README_fr.md)

此文件夹存放用于下载车站数据等任务的 PC 端工具。

## 下载车站数据

### 环境要求
- Python（推荐 Python 3.10 或更高版本）
- 网络连接

无需安装额外的软件包。此脚本仅使用 Python 标准库。

### 运行方法

1. 从 GitHub 下载 [`pc` 分支](https://github.com/marimutukisinnmu-rgb/CardputerADV-DensyaNavi/tree/pc) 的 ZIP 文件。
2. 解压 ZIP 文件。
3. 在包含 `download_data.py` 的文件夹中打开终端，并运行下面对应的命令。

**Windows**

如果可以使用 Python Launcher 的 `py` 命令：

```cmd
py download_data.py
```

或者，如果 Python 已添加到 PATH：

```cmd
python download_data.py
```

命令提示符和 PowerShell 均可使用。

**macOS / Linux**

如果 Python 3 已添加到 PATH，请运行：

```sh
python3 download_data.py
```

### 下载后的目录结构

运行成功后，脚本所在目录旁会创建 `station_data/`，其中包含：

```text
station_data/
├── out/
├── README.md
└── LICENSE.md
```

数据从 [Seo-4d696b75/station_database](https://github.com/Seo-4d696b75/station_database) 的 `main` 分支下载。无需安装 Git。

### 当前脚本的功能
- 从 GitHub 下载 ZIP 文件
- 解压 `out/` 目录
- 保存原项目的 `README.md` 和 `LICENSE.md`

目前不会处理或转换数据，也不会向 microSD 卡写入数据。

### 许可与注意事项

请查看 `station_data/LICENSE.md` 和 `station_data/README.md`，了解源数据的许可证、出处和使用条件。源仓库注明不保证数据的准确性或时效性。
