# CardputerADV-DensyaNavi

Cardputer ADV向けの電車専用ナビゲーションプロジェクトです。

## 目標

- GPSなどから現在地・移動方向を取得
- 現在いる駅や走行中の路線を推定
- 大規模駅では番線入力で列車候補を絞り込み
- 地下区間では時刻表・駅順・センサー情報で追跡を継続
- 次駅、行先、種別、到着予想、遅延情報を表示
- microSD上の路線図・車内案内風PNGを表示
- JR各社、地下鉄、私鉄など、全国の鉄道事業者へ拡張できる構造を目指す

## 対応方針

特定の事業者だけを「全国対応」とはせず、データProviderを追加できる構造にします。

- JR各社
- 地下鉄
- 大手私鉄
- 地方私鉄・第三セクター等
- 利用可能なGTFS / GTFS-Realtime / 公開データ
- 必要に応じて利用条件を確認した公式運行情報

## 開発環境

M5Stack公式のCardputer-Adv向けPlatformIO設定をベースにしています。

```powershell
pio run
```

ビルド成功後、通常は次にアプリケーションバイナリが生成されます。

```text
.pio/build/m5stack-cardputer/firmware.bin
```

## 現在の段階

まずはCardputer ADV上で起動し、画面に `DensyaNavi` と表示する最小構成です。
GPS、microSD、鉄道データ、PNG表示、列車推定は順次追加します。

## License

MIT License
