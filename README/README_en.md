# CardputerADV-DensyaNavi — PC Tools

[日本語](README.md) | [English](README_en.md) | [한국어](README_ko.md) | [繁體中文](README_zh-TW.md) | [简体中文](README_zh-CN.md) | [Español](README_es.md) | [Français](README_fr.md)

This directory contains PC-side tools for tasks such as downloading railway station data.

## Downloading Station Data

### Requirements
- Python (Python 3.10 or later recommended)
- An internet connection

No additional packages are required. The script uses only Python's standard library.

### How to Run

1. Download a ZIP archive of the [`pc` branch](https://github.com/marimutukisinnmu-rgb/CardputerADV-DensyaNavi/tree/pc) from GitHub.
2. Extract the ZIP archive.
3. Open a terminal in the folder containing `download_data.py` and run the appropriate command below.

**Windows**

If the Python Launcher (`py` command) is available:

```cmd
py download_data.py
```

Alternatively, if Python is available on your PATH:

```cmd
python download_data.py
```

Both Command Prompt and PowerShell can be used.

**macOS / Linux**

If Python 3 is available on your PATH, run:

```sh
python3 download_data.py
```

### Output Structure

On success, a `station_data/` directory is created next to the script, containing:

```text
station_data/
├── out/
├── README.md
└── LICENSE.md
```

The data is downloaded from the `main` branch of [Seo-4d696b75/station_database](https://github.com/Seo-4d696b75/station_database). Git does not need to be installed.

### What the Script Currently Does
- Downloads a ZIP archive from GitHub
- Extracts the `out/` directory
- Saves the source project's `README.md` and `LICENSE.md`

It does not currently process or convert the data, or write anything to a microSD card.

### License and Notes

Check `station_data/LICENSE.md` and `station_data/README.md` for the source data's license, attribution, and terms of use. The source repository notes that it does not guarantee the data's accuracy or freshness.
