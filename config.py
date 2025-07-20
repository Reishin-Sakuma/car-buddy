# CarBuddy Python版設定ファイル
# Raspberry Pi Zero 2 W用設定

import os

# === 画面設定 ===
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 480
FULLSCREEN = True

# === 更新間隔（ミリ秒） ===
TEMP_UPDATE_INTERVAL = 1000   # 温度: 1秒（ESP32では2秒だったが高速化）
SPEED_UPDATE_INTERVAL = 50    # 速度: 50ms（ESP32では100msだったが高速化）
TIME_UPDATE_INTERVAL = 1000   # 時刻: 1秒
BACKGROUND_UPDATE_INTERVAL = 100  # 背景色: 100ms（ESP32では500msだったが高速化）
UI_REFRESH_INTERVAL = 16      # UI全体: 約60FPS（新規追加）

# === センサー設定 ===
# 温度センサー（DS18B20）
TEMP_SENSOR_ID = None  # Noneの場合は自動検出

# 加速度センサー（MPU6050）
MPU6050_I2C_BUS = 1
MPU6050_ADDRESS = 0x68

# ロータリーエンコーダー（3ピン）
ROTARY_ENCODER_CLK_PIN = 17  # GPIO17 (ピン11)
ROTARY_ENCODER_DT_PIN = 18   # GPIO18 (ピン12)

# === 画像設定 ===
CHARACTER_IMAGE_DIR = "images"
CHARACTER_IMAGE_FILES = {
    "normal": "character_normal.png",
    "hot": "character_hot.png",
    "wink": "character_wink.png"
}

# === フォント設定 ===
FONTS = {
    "temperature": ("Arial", 72, "bold"),      # 温度表示（ESP32では小さかった）
    "speed": ("Arial", 48, "normal"),          # 速度表示
    "time": ("Arial", 36, "bold"),             # 時刻表示
    "date": ("Arial", 36, "normal"),           # 日付表示（時刻と同じサイズに）
    "splash": ("Arial", 32, "bold"),           # スプラッシュ画面
    "status": ("Arial", 16, "normal")          # ステータス表示
}

# === 色設定 ===
COLORS = {
    "background_cold": "#0066CC",  # 青
    "background_hot": "#CC0000",   # 赤
    "text_normal": "#FFFFFF",      # 白
    "text_hot": "#FFFF00",         # 黄色
    "text_time": "#FFFF00",        # 黄色
    "text_date": "#00FFFF",        # シアン
    "text_shadow": "#000000"       # 影（新規追加）
}

# === アニメーション設定 ===
ANIMATIONS = {
    "background_transition_duration": 300,  # 背景色変化時間（ms）
    "text_fade_duration": 200,              # テキストフェード時間（ms）
    "character_change_duration": 150,       # キャラ変更時間（ms）
    "smooth_value_change": True,            # 数値の滑らかな変化
    "easing_enabled": True,                 # イージング効果有効
}

# === 温度閾値 ===
TEMP_THRESHOLDS = {
    "hot_warning": 32.0,    # 警告温度
    "transition": 30.0,     # 背景色変化開始
    "background_change": 0.5  # 背景変更の最小温度差（ESP32では1.0だったが高感度化）
}

# === Webサーバー設定 ===
WEB_SERVER_ENABLED = True
WEB_SERVER_PORT = 80
WEB_SERVER_HOST = "0.0.0.0"

# === ログ設定 ===
LOG_LEVEL = "INFO"
LOG_FILE = "carbuddy.log"

# === GPIO設定（将来的なロータリーエンコーダー用） ===
ROTARY_ENCODER_PINS = {
    "clk": 17,
    "dt": 18,
    "sw": 27
}

# === デバッグ設定 ===
DEBUG_MODE = False
MOCK_SENSORS = True  # センサーなしでテスト実行