# CarBuddy Python版 - 温度センサー管理
import logging
import time
from typing import Optional

try:
    from w1thermsensor import W1ThermSensor
    W1_AVAILABLE = True
except ImportError:
    W1_AVAILABLE = False
    logging.warning("w1thermsensor not available - running in mock mode")

logger = logging.getLogger(__name__)

class TemperatureSensor:
    """DS18B20温度センサー管理クラス"""
    
    def __init__(self, config: dict):
        self.config = config
        self.sensor: Optional[W1ThermSensor] = None
        self.mock_mode = config.get("MOCK_SENSORS", False) or not W1_AVAILABLE
        self.last_temperature = 20.0  # デフォルト温度
        self.mock_temperature = 20.0  # モック用温度
        
    def initialize(self) -> bool:
        """温度センサーを初期化"""
        if self.mock_mode:
            logger.info("Temperature sensor initialized in mock mode")
            return True
            
        try:
            sensor_id = self.config.get("TEMP_SENSOR_ID")
            
            if sensor_id:
                # 特定のセンサーIDを指定
                self.sensor = W1ThermSensor(sensor_id=sensor_id)
            else:
                # 最初に見つかったセンサーを使用
                self.sensor = W1ThermSensor()
            
            # 初回読み取りテスト
            test_temp = self.sensor.get_temperature()
            logger.info(f"Temperature sensor initialized successfully. Test reading: {test_temp:.1f}°C")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize temperature sensor: {e}")
            logger.info("Falling back to mock mode")
            self.mock_mode = True
            return True  # モックモードで継続
    
    def get_temperature(self) -> float:
        """現在の温度を取得（°C）"""
        if self.mock_mode:
            return self._get_mock_temperature()
        
        try:
            temperature = self.sensor.get_temperature()
            self.last_temperature = temperature
            return temperature
            
        except Exception as e:
            logger.error(f"Failed to read temperature: {e}")
            # エラー時は前回値を返す
            return self.last_temperature
    
    def _get_mock_temperature(self) -> float:
        """モック用温度データ生成"""
        import math
        
        # 時間に応じてゆっくり変化する温度をシミュレート
        time_factor = time.time() * 0.01  # ゆっくりとした変化
        temperature_variation = math.sin(time_factor) * 5.0  # ±5度の変化
        base_temperature = 25.0
        
        self.mock_temperature = base_temperature + temperature_variation
        
        # たまに高温をシミュレート（テスト用）
        if int(time.time()) % 30 < 5:  # 30秒中5秒間は高温
            self.mock_temperature += 10.0
        
        return self.mock_temperature
    
    def is_available(self) -> bool:
        """センサーが利用可能かチェック"""
        if self.mock_mode:
            return True
        return self.sensor is not None
    
    def get_sensor_info(self) -> dict:
        """センサー情報を取得"""
        if self.mock_mode:
            return {
                "type": "Mock",
                "id": "mock_sensor",
                "status": "active"
            }
        
        if self.sensor:
            return {
                "type": "DS18B20",
                "id": getattr(self.sensor, 'id', 'unknown'),
                "status": "active"
            }
        
        return {
            "type": "Unknown",
            "id": "none",
            "status": "inactive"
        }