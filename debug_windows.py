# CarBuddy Python版 - Windows デバッグ用設定

# Windows用の設定オーバーライド
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600  # Windows用に少し高く
FULLSCREEN = False   # ウィンドウモードで起動

# 更新間隔（デバッグ用に高速化）
TEMP_UPDATE_INTERVAL = 1000   # 温度: 1秒
SPEED_UPDATE_INTERVAL = 200   # 速度: 200ms
TIME_UPDATE_INTERVAL = 1000   # 時刻: 1秒
BACKGROUND_UPDATE_INTERVAL = 300  # 背景色: 300ms

# センサー設定（Windows用はモックモード強制）
TEMP_SENSOR_ID = None
MPU6050_I2C_BUS = 1
MPU6050_ADDRESS = 0x68

# 画像設定
CHARACTER_IMAGE_DIR = "images"
CHARACTER_IMAGE_FILES = {
    "normal": "character_normal.png",
    "hot": "character_hot.png",
    "wink": "character_wink.png"
}

# 色設定
COLORS = {
    "background_cold": "#0066CC",  # 青
    "background_hot": "#CC0000",   # 赤
    "text_normal": "#FFFFFF",      # 白
    "text_hot": "#FFFF00",         # 黄色
    "text_time": "#FFFF00",        # 黄色
    "text_date": "#00FFFF"         # シアン
}

# 温度閾値
TEMP_THRESHOLDS = {
    "hot_warning": 32.0,    # 警告温度
    "transition": 30.0,     # 背景色変化開始
    "background_change": 1.0  # 背景変更の最小温度差
}

# Webサーバー設定
WEB_SERVER_ENABLED = False  # Windows用は無効
WEB_SERVER_PORT = 80
WEB_SERVER_HOST = "0.0.0.0"

# ログ設定
LOG_LEVEL = "DEBUG"  # デバッグ用
LOG_FILE = "carbuddy_debug.log"

# GPIO設定（将来的なロータリーエンコーダー用）
ROTARY_ENCODER_PINS = {
    "clk": 17,
    "dt": 18,
    "sw": 27
}

# デバッグ設定
DEBUG_MODE = True
MOCK_SENSORS = True  # Windows用は強制的にモックモード