# 🚗 car-buddy（しゃべるモニター）

ESP32ベースのダッシュボード用スマートガジェット。温度・速度表示、AI音声、タッチ操作に対応した多機能デバイスです。

## 🧹 概要

* 2.8インチタッチ液晶 + ESP32
* DS18B20温度センサー
* 加速度センサーで速度検出（予定）
* microSDから音声ファイル再生（予定）
* レースクイーン風AIボイス搭載（予定）

## 📦 使用パーツ（例）

* ESP32 DevKitC-VE
* ILI9341 2.8インチタッチ液晶（SPI）
* PAM8403 ステレオD等アンプモジュール
* スピーカー（5cm/8Ω）
* DS18B20 温度センサー
* microSD（液晶側に内蔵）

## ⚙️ 開発環境

* VSCode + PlatformIO
* フレームワーク：Arduino
* PlatformIOプロジェクト名：`car-buddy`

## 🚀 セットアップ手順

```bash
# 1. リポジトリをクローン
git clone https://github.com/yourname/car-buddy.git
cd car-buddy

# 2. PlatformIOで開く（VSCode推奨）

# 3. ビルド & アップロード
pio run --target upload
```

## 🔧 PlatformIO 設定例（`platformio.ini`

```ini
[env:car-buddy]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

lib_deps =
  bodmer/TFT_eSPI
  paulstoffregen/OneWire
  milesburton/DallasTemperature
  adafruit/Adafruit VS1053 Library@^1.2.1 ; （未使用なら削除）

build_flags =
  -DUSER_SETUP_LOADED
  -DTFT_CS=5
  -DTFT_DC=21
  -DTFT_RST=22
  -DTFT_MOSI=23
  -DTFT_SCLK=18
  -DSDCARD_CS=4

upload_speed = 921600
```

## 🖼️ UIレイアウト（仮）

```
|-------------------------|
| キャラクター画像        | 温度表示（右上）
|                         |----------------
|                         |                |
|                         | 速度表示（右下）|
|-------------------------|
```

## 📁 ディレクトリ構成（例）

```
car-buddy/
├── lib/
│   └── display/         # 画面描画関連
├── src/
│   └── main.cpp         # エントリポイント
├── include/
│   └── settings.h       # 各種定義
├── platformio.ini
└── README.md
```

## 📜 ライセンス

MIT License

---

## 🙌 Special Thanks

* PlatformIO
* TFT\_eSPI
* ChatGPT ✨
