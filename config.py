# CarBuddy Python版設定ファイル
# Raspberry Pi Zero 2 W用設定

import os

# === 画面設定 ===
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 480
FULLSCREEN = True

# === 更新間隔（ミリ秒） ===
TEMP_UPDATE_INTERVAL = 2000   # 温度: 2秒
SPEED_UPDATE_INTERVAL = 100   # 速度: 100ms
TIME_UPDATE_INTERVAL = 1000   # 時刻: 1秒
BACKGROUND_UPDATE_INTERVAL = 500  # 背景色: 500ms

# === センサー設定 ===
# 温度センサー（DS18B20）
TEMP_SENSOR_ID = None  # Noneの場合は自動検出

# 加速度センサー（MPU6050）
MPU6050_I2C_BUS = 1
MPU6050_ADDRESS = 0x68

# === 画像設定 ===
CHARACTER_IMAGE_DIR = "images"
CHARACTER_IMAGE_FILES = {
    "normal": "character_normal.png",
    "hot": "character_hot.png",
    "wink": "character_wink.png"
}

# === 色設定 ===
COLORS = {
    "background_cold": "#0066CC",  # 青
    "background_hot": "#CC0000",   # 赤
    "text_normal": "#FFFFFF",      # 白
    "text_hot": "#FFFF00",         # 黄色
    "text_time": "#FFFF00",        # 黄色
    "text_date": "#00FFFF"         # シアン
}

# === 温度閾値 ===
TEMP_THRESHOLDS = {
    "hot_warning": 32.0,    # 警告温度
    "transition": 30.0,     # 背景色変化開始
    "background_change": 1.0  # 背景変更の最小温度差
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
MOCK_SENSORS = False  # センサーなしでテスト実行