# 🚗 car-buddy - Talking Monitor

ESP32ベースの車載インタラクティブディスプレイシステム。温度と加速度をリアルタイム表示し、美しいUI演出でデータを可視化します。

![car-buddy Demo](https://img.shields.io/badge/Status-Active-green) ![ESP32](https://img.shields.io/badge/Platform-ESP32-blue) ![PlatformIO](https://img.shields.io/badge/IDE-PlatformIO-orange)

## ✨ 主な機能

- **🎬 プロ仕様の起動演出** - スプラッシュ画面とフェードイン効果
- **🌡️ リアルタイム温度監視** - DS18B20センサーによる精密測定
- **📈 加速度表示** - MPU6500/6050による3軸加速度センサー
- **🎨 美しいUI** - 320x240 TFTディスプレイでキャラクター表示
   - UIイメージ(現実はこんなにモダンではありません)：https://claude.ai/public/artifacts/7297501f-ceec-4aa7-88c3-ab68484830fa
- **⚡ 最適化されたパフォーマンス** - 差分描画によるちらつき防止

## 🔧 ハードウェア構成

| コンポーネント | 型番/仕様 | 接続ピン |
|---|---|---|
| **マイコン** | ESP32 Dev Module | - |
| **温度センサー** | DS18B20 | GPIO25 |
| **加速度センサー** | MPU6500/6050 | I2C (SDA: GPIO21, SCL: GPIO22) |
| **ディスプレイ** | 1.8インチ TFT LCD (320x240) | TFT_eSPI設定 |

### 🔌 接続図

```
ESP32 Dev Module
├── GPIO25 ──── DS18B20 (温度センサー)
├── GPIO21 ──── SDA (MPU6500)
├── GPIO22 ──── SCL (MPU6500)
└── TFT Pins ── 1.8" TFT Display
```

## 🚀 セットアップ

### 必要なソフトウェア

- [PlatformIO](https://platformio.org/) (VS Code拡張推奨)
- Arduino framework for ESP32

### ライブラリ依存関係

```ini
lib_deps = 
    OneWire@^2.3.8
    DallasTemperature@^3.11.0
    TFT_eSPI@^2.5.43
    Adafruit MPU6050@^2.2.6
    Adafruit Unified Sensor@^1.1.15
```

### インストール手順

1. **リポジトリをクローン**
   ```bash
   git clone https://github.com/yourusername/car-buddy.git
   cd car-buddy
   ```

2. **PlatformIOでプロジェクトを開く**
   ```bash
   pio project init
   ```

3. **TFT_eSPI設定**
   - `User_Setup.h`でディスプレイ設定を確認
   - 使用するTFTディスプレイに合わせてピン設定を調整

4. **ビルド & アップロード**
   ```bash
   pio run --target upload
   ```

## 📁 プロジェクト構成

```
car-buddy/
├── src/
│   ├── main.cpp           # メインプログラム
│   ├── ui.cpp/hpp         # UI描画とフェード効果
│   ├── temperature.cpp/hpp # 温度センサー管理
│   ├── speed.cpp/hpp      # 加速度センサー管理
│   └── characterImage.h   # キャラクター画像データ
├── platformio.ini         # PlatformIO設定
└── README.md
```

## 🎮 使用方法

### 起動シーケンス

1. **スプラッシュ画面** - "car-buddy" ロゴがフェードイン・アウト
2. **メイン画面** - 背景とUIがフェードイン
3. **キャラクター表示** - 縁ぼかし効果付きで登場
4. **データ更新開始** - リアルタイム監視モード

### 表示内容

- **温度**: 2秒間隔で更新（°C表示）
- **速度**: 100ms間隔で加速度値を表示（将来的に時速計算対応予定）
- **キャラクター**: 180x180サイズでメイン表示

## ⚙️ カスタマイズ

### 更新間隔の調整

```cpp
// main.cpp内
const unsigned long TEMP_UPDATE_INTERVAL = 2000;  // 温度更新間隔
const unsigned long SPEED_UPDATE_INTERVAL = 100;   // 速度更新間隔
```

### フェード効果の調整

```cpp
// ui.cpp内
const int fadeWidth = 8;        // 縁フェード幅
float alpha = 0.3 + 0.7 * ...   // フェード強度
```

### センサー設定

```cpp
// temperature.cpp
#define ONE_WIRE_BUS 25         // 温度センサーピン

// speed.cpp  
Wire.begin(21, 22);             // I2Cピン設定
```

## 🔍 トラブルシューティング

### センサーが認識されない

```bash
# シリアルモニターで確認
Scanning I2C devices...
I2C device found at address 0x68  # MPU6500/6050
Found 1 temperature sensor(s)     # DS18B20
```

### ディスプレイが表示されない

1. TFT_eSPIライブラリの設定を確認
2. ピン接続を再確認
3. 電源供給を確認

### コンパイルエラー

- ライブラリ依存関係を確認
- PlatformIOライブラリを更新: `pio lib update`

## 🛣️ 今後の予定

- [ ] **音声出力機能** - レースクイーン風ボイス
- [ ] **WiFi連携** - データログ・リモート監視
- [ ] **速度計算** - 加速度積分による実時速表示
- [ ] **アラート機能** - 温度・速度閾値通知
- [ ] **データロガー** - SDカード記録機能

## 🤝 コントリビューション

プルリクエストや Issue の報告を歓迎します！

1. このリポジトリをフォーク
2. フィーチャーブランチを作成 (`git checkout -b feature/AmazingFeature`)
3. 変更をコミット (`git commit -m 'Add some AmazingFeature'`)
4. ブランチにプッシュ (`git push origin feature/AmazingFeature`)
5. プルリクエストを作成

## 📄 ライセンス

このプロジェクトは MIT ライセンスの下で公開されています。詳細は [LICENSE](LICENSE) ファイルを参照してください。

## 🙏 謝辞

- ESP32 コミュニティ
- TFT_eSPI ライブラリ開発者
- Adafruit センサーライブラリ

---

**⭐ このプロジェクトが役に立ったら、スターをお願いします！**