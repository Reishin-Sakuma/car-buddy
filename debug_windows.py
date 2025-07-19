# CarBuddy Python版 - Windows デバッグ用設定

# Windows用の設定オーバーライド
SCREEN_WIDTH = 800
SCREEN_HEIGHT = 600  # Windows用に少し高く
FULLSCREEN = False   # ウィンドウモードで起動

# 更新間隔（本番と同じ高性能設定）
TEMP_UPDATE_INTERVAL = 1000   # 温度: 1秒
SPEED_UPDATE_INTERVAL = 50    # 速度: 50ms（本番と同じ高速）
TIME_UPDATE_INTERVAL = 1000   # 時刻: 1秒
BACKGROUND_UPDATE_INTERVAL = 100  # 背景色: 100ms（本番と同じ高速）
UI_REFRESH_INTERVAL = 16      # UI全体: 約60FPS

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

# フォント設定（本番と同じ大型フォント）
FONTS = {
    "temperature": ("Arial", 72, "bold"),      # 温度表示
    "speed": ("Arial", 48, "normal"),          # 速度表示
    "time": ("Arial", 36, "bold"),             # 時刻表示
    "date": ("Arial", 36, "normal"),           # 日付表示（時刻と同じサイズに）
    "splash": ("Arial", 32, "bold"),           # スプラッシュ画面
    "status": ("Arial", 16, "normal")          # ステータス表示
}

# 色設定
COLORS = {
    "background_cold": "#0066CC",  # 青
    "background_hot": "#CC0000",   # 赤
    "text_normal": "#FFFFFF",      # 白
    "text_hot": "#FFFF00",         # 黄色
    "text_time": "#FFFF00",        # 黄色
    "text_date": "#00FFFF",        # シアン
    "text_shadow": "#000000"       # 影
}

# アニメーション設定（本番と同じ高品質設定）
ANIMATIONS = {
    "background_transition_duration": 300,  # 背景色変化時間
    "text_fade_duration": 200,              # テキストフェード時間
    "character_change_duration": 150,       # キャラ変更時間
    "smooth_value_change": True,            # 数値の滑らかな変化
    "easing_enabled": True,                 # イージング効果有効
}

# 温度閾値
TEMP_THRESHOLDS = {
    "hot_warning": 32.0,    # 警告温度
    "transition": 30.0,     # 背景色変化開始
    "background_change": 0.5  # 背景変更の最小温度差（高感度化）
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