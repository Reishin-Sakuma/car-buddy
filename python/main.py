# CarBuddy Python版 - メインアプリケーション
# Raspberry Pi Zero 2 W + HDMI出力版

import logging
import threading
import time
import sys
import os
from typing import Optional

# 設定ファイルをインポート
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
import config

# 各モジュールをインポート
from ui.display_manager import DisplayManager
from sensors.temperature_sensor import TemperatureSensor
from sensors.speed_sensor import SpeedSensor
from utils.time_manager import TimeManager

# ログ設定
logging.basicConfig(
    level=getattr(logging, config.LOG_LEVEL),
    format='%(asctime)s - %(name)s - %(levelname)s - %(message)s',
    handlers=[
        logging.FileHandler(config.LOG_FILE),
        logging.StreamHandler()
    ]
)

logger = logging.getLogger(__name__)

class CarBuddyApp:
    """CarBuddy メインアプリケーションクラス"""
    
    def __init__(self):
        self.config = {
            "SCREEN_WIDTH": config.SCREEN_WIDTH,
            "SCREEN_HEIGHT": config.SCREEN_HEIGHT,
            "FULLSCREEN": config.FULLSCREEN,
            "CHARACTER_IMAGE_DIR": config.CHARACTER_IMAGE_DIR,
            "CHARACTER_IMAGE_FILES": config.CHARACTER_IMAGE_FILES,
            "COLORS": config.COLORS,
            "TEMP_THRESHOLDS": config.TEMP_THRESHOLDS,
            "TEMP_UPDATE_INTERVAL": config.TEMP_UPDATE_INTERVAL,
            "SPEED_UPDATE_INTERVAL": config.SPEED_UPDATE_INTERVAL,
            "TIME_UPDATE_INTERVAL": config.TIME_UPDATE_INTERVAL,
            "BACKGROUND_UPDATE_INTERVAL": config.BACKGROUND_UPDATE_INTERVAL,
            "MOCK_SENSORS": config.MOCK_SENSORS,
            "MPU6050_I2C_BUS": config.MPU6050_I2C_BUS,
            "MPU6050_ADDRESS": config.MPU6050_ADDRESS,
            "TEMP_SENSOR_ID": config.TEMP_SENSOR_ID
        }
        
        # コンポーネント初期化
        self.display_manager: Optional[DisplayManager] = None
        self.temperature_sensor: Optional[TemperatureSensor] = None
        self.speed_sensor: Optional[SpeedSensor] = None
        self.time_manager: Optional[TimeManager] = None
        
        # 更新スレッド管理
        self.running = False
        self.update_threads = []
        
        # 前回の値（差分更新用）
        self.last_values = {
            "temperature": None,
            "speed": None,
            "time": None,
            "date": None
        }
    
    def initialize(self) -> bool:
        """アプリケーション初期化"""
        logger.info("=== CarBuddy Python Edition Starting ===")
        
        try:
            # ディスプレイ管理の初期化
            self.display_manager = DisplayManager(self.config)
            if not self.display_manager.initialize_display():
                logger.error("Failed to initialize display")
                return False
            
            # センサー初期化
            self.temperature_sensor = TemperatureSensor(self.config)
            if not self.temperature_sensor.initialize():
                logger.error("Failed to initialize temperature sensor")
                return False
            
            self.speed_sensor = SpeedSensor(self.config)
            if not self.speed_sensor.initialize():
                logger.error("Failed to initialize speed sensor")
                return False
            
            # 時刻管理初期化
            self.time_manager = TimeManager(self.config)
            
            logger.info("All components initialized successfully")
            
            # スプラッシュ画面表示
            self.display_manager.show_splash_screen()
            
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize application: {e}")
            return False
    
    def start_update_threads(self):
        """更新スレッドを開始"""
        self.running = True
        
        # 温度更新スレッド
        temp_thread = threading.Thread(target=self._temperature_update_loop, daemon=True)
        temp_thread.start()
        self.update_threads.append(temp_thread)
        
        # 速度更新スレッド
        speed_thread = threading.Thread(target=self._speed_update_loop, daemon=True)
        speed_thread.start()
        self.update_threads.append(speed_thread)
        
        # 時刻更新スレッド
        time_thread = threading.Thread(target=self._time_update_loop, daemon=True)
        time_thread.start()
        self.update_threads.append(time_thread)
        
        logger.info("Update threads started")
    
    def _temperature_update_loop(self):
        """温度更新ループ"""
        while self.running:
            try:
                temperature = self.temperature_sensor.get_temperature()
                
                # 差分更新
                if self.last_values["temperature"] != temperature:
                    self.display_manager.update_temperature_display(temperature)
                    self.last_values["temperature"] = temperature
                    logger.debug(f"Temperature updated: {temperature:.1f}°C")
                
            except Exception as e:
                logger.error(f"Temperature update error: {e}")
            
            time.sleep(self.config["TEMP_UPDATE_INTERVAL"] / 1000.0)
    
    def _speed_update_loop(self):
        """速度更新ループ"""
        while self.running:
            try:
                speed = self.speed_sensor.get_speed()
                
                # 差分更新（小数点1位で比較）
                if self.last_values["speed"] is None or abs(self.last_values["speed"] - speed) > 0.1:
                    self.display_manager.update_speed_display(speed)
                    self.last_values["speed"] = speed
                    logger.debug(f"Speed updated: {speed:.1f} km/h")
                
            except Exception as e:
                logger.error(f"Speed update error: {e}")
            
            time.sleep(self.config["SPEED_UPDATE_INTERVAL"] / 1000.0)
    
    def _time_update_loop(self):
        """時刻更新ループ"""
        while self.running:
            try:
                time_str = self.time_manager.get_current_time()
                date_str = self.time_manager.get_current_date()
                
                # 時刻更新
                if self.last_values["time"] != time_str:
                    self.display_manager.update_time_display(time_str)
                    self.last_values["time"] = time_str
                
                # 日付更新
                if self.last_values["date"] != date_str:
                    self.display_manager.update_date_display(date_str)
                    self.last_values["date"] = date_str
                
            except Exception as e:
                logger.error(f"Time update error: {e}")
            
            time.sleep(self.config["TIME_UPDATE_INTERVAL"] / 1000.0)
    
    def stop(self):
        """アプリケーション停止"""
        logger.info("Stopping CarBuddy application...")
        self.running = False
        
        # スレッド終了待機
        for thread in self.update_threads:
            thread.join(timeout=1.0)
        
        # センサークリーンアップ
        if self.speed_sensor:
            self.speed_sensor.cleanup()
        
        # ディスプレイクリーンアップ
        if self.display_manager:
            self.display_manager.destroy()
        
        logger.info("CarBuddy application stopped")
    
    def run(self):
        """メインアプリケーション実行"""
        try:
            if not self.initialize():
                return False
            
            # 更新スレッド開始
            self.start_update_threads()
            
            # メインループ（GUIイベントループ）
            logger.info("Starting main GUI loop")
            self.display_manager.run_main_loop()
            
        except KeyboardInterrupt:
            logger.info("Received keyboard interrupt")
        except Exception as e:
            logger.error(f"Application error: {e}")
        finally:
            self.stop()
        
        return True

def main():
    """エントリーポイント"""
    app = CarBuddyApp()
    success = app.run()
    
    if success:
        logger.info("CarBuddy application completed successfully")
        sys.exit(0)
    else:
        logger.error("CarBuddy application failed")
        sys.exit(1)

if __name__ == "__main__":
    main()