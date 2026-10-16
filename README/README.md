# CardputerADV-DensyaNavi — PC tools

[日本語](README.md) | [English](README_en.md) | [한국어](README_ko.md) | [繁體中文](README_zh-TW.md) | [简体中文](README_zh-CN.md) | [Español](README_es.md) | [Français](README_fr.md)

PC側で駅データの取得などを行うためのツール置き場です。

## 駅データのダウンロード

### 必要なもの
- Python（Python 3.10 以降を推奨）
- インターネット接続

追加ライブラリのインストールは不要です。Python標準ライブラリだけで動作します。

### 実行方法

1. GitHubの [`pc` ブランチ](https://github.com/marimutukisinnmu-rgb/CardputerADV-DensyaNavi/tree/pc) からZIPをダウンロードします。
2. ZIPを展開します。
3. `download_data.py` があるフォルダを作業フォルダにして、以下のコマンドを実行します。

**Windows**

Python Launcher（`py` コマンド）が使える場合：

```cmd
py download_data.py
```

または、PythonにPATH（環境変数）が通っている場合：

```cmd
python download_data.py
```

※ コマンドプロンプトとPowerShellのどちらでも実行できます。

**macOS / Linux**

Python 3にPATHが通っている状態で、次を実行します。

```sh
python3 download_data.py
```

### ダウンロード後の構成

成功すると、スクリプトと同じ階層に `station_data/` が作成され、次の内容が保存されます。

```text
station_data/
├── out/
├── README.md
└── LICENSE.md
```

取得元は [Seo-4d696b75/station_database](https://github.com/Seo-4d696b75/station_database) の `main` ブランチです。Gitをインストールする必要はありません。

### 現時点で行うこと
- GitHub ZIPのダウンロード
- `out/` ディレクトリの展開
- 元データの `README.md` と `LICENSE.md` の保存

データの加工、変換、microSDへの書き込みはまだ行いません。

### ライセンスと注意

取得元データのライセンス・出典・利用条件は、保存された `station_data/LICENSE.md` と `station_data/README.md` を確認してください。取得元リポジトリには、データの正確性や最新性を保証しない旨の注意があります。
