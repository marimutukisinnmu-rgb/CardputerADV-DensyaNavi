# CardputerADV-DensyaNavi — PC tools

PC側で駅データの取得などを行うためのツール置き場です。

## 駅データのダウンロード

### 必要なもの
- Python(Python 3.10 以降を推奨)
- インターネット接続

追加ライブラリのインストールは不要です。Python標準ライブラリだけで動作します。

### 実行方法

このブランチをPCに取得し、リポジトリ直下で次を実行します。

Windows:
pythonをPATH(環境変数)に通した状態かつ今の作業フォルダがダウンロードした場所で
```cmd
py download_data.py
```
または
```cmd
python download_data.py
```
`※powershellでも可`
macOS / Linux:
同じくPATH(環境変数)を通している状態で
```sh
python3 download_data.py
```

成功すると、スクリプトと同じ階層に `station_data/` が作成され、次の内容が保存されます。

```tree
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
