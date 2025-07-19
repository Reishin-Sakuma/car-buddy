# 🚗 CarBuddy Python版 - Raspberry Pi Zero 2 W対応

ESP32版からPython + Raspberry Pi Zero 2 W版に移植された車載インタラクティブディスプレイシステム。HDMI出力でモニターに表示し、温度と加速度をリアルタイム監視します。

![CarBuddy Python](https://img.shields.io/badge/Status-Active-green) ![Raspberry%20Pi](https://img.shields.io/badge/Platform-Raspberry%20Pi-red) ![Python](https://img.shields.io/badge/Language-Python-blue)

## ✨ 主な機能

- **🎬 プロ仕様の起動演出** - スプラッシュ画面とフェードイン効果
- **🌡️ リアルタイム温度監視** - DS18B20センサーによる精密測定
- **📈 加速度表示** - MPU6050による3軸加速度センサー
- **🎨 美しいtkinter GUI** - HDMI出力でフルスクリーン表示
- **🖼️ PNG画像対応** - キャラクター画像を自由にカスタマイズ
- **⚡ マルチスレッド処理** - 軽量で安定した動作

## 🔧 ハードウェア構成

| コンポーネント | 型番/仕様 | 接続 |
|---|---|---|
| **SBC** | Raspberry Pi Zero 2 W | - |
| **温度センサー** | DS18B20 | GPIO（1-Wire） |
| **加速度センサー** | MPU6050 | I2C (SDA: GPIO2, SCL: GPIO3) |
| **ディスプレイ** | HDMIモニター | HDMI出力 |

### 🔌 接続図

```
Raspberry Pi Zero 2 W
├── GPIO2 (SDA) ──── MPU6050
├── GPIO3 (SCL) ──── MPU6050  
├── GPIO4 ──────── DS18B20 (Data)
├── 3.3V ──────── センサー電源
├── GND ───────── GND
└── HDMI ──────── モニター
```

## 🚀 セットアップ

### 必要なソフトウェア

- Raspberry Pi OS (Bullseye以降推奨)
- Python 3.9以降
- 1-Wire及びI2Cの有効化

### システム準備

1. **Raspberry Pi設定**
   ```bash
   sudo raspi-config
   # Interface Options → I2C → Enable
   # Interface Options → 1-Wire → Enable
   ```

2. **1-Wireデバイス確認**
   ```bash
   ls /sys/bus/w1/devices/
   # 28-xxxxxxxxxx が表示されればDS18B20が認識されている
   ```

3. **I2Cデバイス確認**
   ```bash
   sudo apt install i2c-tools
   i2cdetect -y 1
   # 0x68 が表示されればMPU6050が認識されている
   ```

### インストール手順

1. **リポジトリをクローン**
   ```bash
   git clone https://github.com/yourusername/car-buddy.git
   cd car-buddy
   ```

2. **Python依存関係をインストール**
   ```bash
   cd python
   pip install -r ../requirements.txt
   ```

3. **キャラクター画像を配置**
   ```bash
   # images/フォルダーに以下を配置
   # - character_normal.png (180x180推奨)
   # - character_hot.png (180x180推奨)
   ```

4. **実行**
   ```bash
   python main.py
   ```

## 📁 プロジェクト構成

```
car-buddy/
├── python/
│   ├── main.py                # メインアプリケーション
│   ├── config.py              # 設定ファイル  
│   ├── ui/
│   │   └── display_manager.py # GUI画面管理
│   ├── sensors/
│   │   ├── temperature_sensor.py # 温度センサー
│   │   └── speed_sensor.py       # 加速度センサー
│   ├── utils/
│   │   └── time_manager.py    # 時刻管理
│   └── tests/                 # テストファイル
├── images/                    # キャラクター画像
├── requirements.txt           # Python依存関係
└── config.py                 # グローバル設定
```

## 🎮 使用方法

### 起動シーケンス

1. **スプラッシュ画面** - "CarBuddy Starting..." 表示
2. **メイン画面** - GUI要素がフェードイン
3. **キャラクター表示** - PNG画像がロード・表示
4. **データ更新開始** - リアルタイム監視モード

### 表示内容

- **温度**: 2秒間隔で更新（°C表示、32°C以上で警告色）
- **速度**: 100ms間隔で加速度値を表示（km/h換算）
- **時刻/日付**: 1秒間隔で更新
- **キャラクター**: 温度に応じて画像切り替え

### 操作方法

- **フルスクリーン終了**: Escキー
- **アプリ終了**: Ctrl+C または ウィンドウ閉じる

## ⚙️ カスタマイズ

### 更新間隔の調整

```python
# config.py内
TEMP_UPDATE_INTERVAL = 2000    # 温度更新間隔（ms）
SPEED_UPDATE_INTERVAL = 100    # 速度更新間隔（ms）
TIME_UPDATE_INTERVAL = 1000    # 時刻更新間隔（ms）
```

### 画面サイズの調整

```python
# config.py内
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 480
FULLSCREEN = True
```

### 色設定の変更

```python
# config.py内
COLORS = {
    "background_cold": "#0066CC",  # 低温時背景色
    "background_hot": "#CC0000",   # 高温時背景色
    "text_normal": "#FFFFFF",      # 通常文字色
    "text_hot": "#FFFF00",         # 高温時文字色
}
```

### センサー設定

```python
# config.py内
TEMP_SENSOR_ID = None          # DS18B20 ID（Noneで自動検出）
MPU6050_I2C_BUS = 1           # I2Cバス番号
MPU6050_ADDRESS = 0x68        # MPU6050アドレス
```

## 🔍 トラブルシューティング

### センサーが認識されない

```bash
# 1-Wire確認
ls /sys/bus/w1/devices/
sudo modprobe w1-gpio
sudo modprobe w1-therm

# I2C確認  
i2cdetect -y 1
sudo modprobe i2c-dev
```

### 画面が表示されない

1. HDMI接続を確認
2. Raspberry Piの画面設定を確認
3. フルスクリーンモードをOFFに設定

### 権限エラー

```bash
# GPIO/I2C権限追加
sudo usermod -a -G gpio,i2c pi
# 再ログイン後に実行
```

### モックモードでのテスト

```python
# config.py内
MOCK_SENSORS = True  # センサーなしでテスト実行
```

## 🧪 テスト実行

```bash
cd python
python -m pytest tests/ -v
```

## 📊 性能最適化

- **軽量化**: tkinterによる軽量GUI
- **マルチスレッド**: センサー読み取りとGUIを分離
- **差分更新**: 変化時のみ画面更新
- **Raspberry Pi最適化**: Zero 2 W向けに調整済み

## 🛣️ 今後の予定

- [ ] **音声出力機能** - 車載音声アラート
- [ ] **WiFi Webインターフェース** - リモート監視
- [ ] **データロガー** - CSVファイル記録
- [ ] **GPIO拡張** - ロータリーエンコーダー対応
- [ ] **アナログ時計モード** - 時計表示切り替え

## 💻 開発者向け情報

### テスト駆動開発

このプロジェクトはTDD原則に従って開発されています：

```bash
# テスト実行
python -m pytest tests/ -v

# カバレッジ確認
pip install pytest-cov
python -m pytest tests/ --cov=. --cov-report=html
```

### モジュール構成

- **display_manager**: tkinter GUI管理
- **temperature_sensor**: DS18B20制御
- **speed_sensor**: MPU6050制御  
- **time_manager**: 時刻処理

## 🤝 コントリビューション

プルリクエストやIssueの報告を歓迎します！

1. このリポジトリをフォーク
2. フィーチャーブランチを作成 (`git checkout -b feature/PythonFeature`)
3. 変更をコミット (`git commit -m 'Add Python feature'`)
4. ブランチにプッシュ (`git push origin feature/PythonFeature`)
5. プルリクエストを作成

## 📄 ライセンス

このプロジェクトは MIT ライセンスの下で公開されています。

## 🙏 謝辞

- Raspberry Pi Foundation
- Python GUI/センサーライブラリ開発者
- オリジナルESP32版の貢献者

---

**⭐ Python版が役に立ったら、スターをお願いします！**