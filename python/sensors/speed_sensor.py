# CarBuddy Python版 - 加速度/速度センサー管理
import logging
import time
import math
from typing import Optional, Tuple

try:
    import smbus2
    SMBUS_AVAILABLE = True
except ImportError:
    SMBUS_AVAILABLE = False
    logging.warning("smbus2 not available - running in mock mode")

logger = logging.getLogger(__name__)

class SpeedSensor:
    """MPU6050加速度センサー管理クラス"""
    
    def __init__(self, config: dict):
        self.config = config
        self.bus: Optional[smbus2.SMBus] = None
        self.mock_mode = config.get("MOCK_SENSORS", False) or not SMBUS_AVAILABLE
        self.i2c_bus = config.get("MPU6050_I2C_BUS", 1)
        self.device_address = config.get("MPU6050_ADDRESS", 0x68)
        
        # センサー設定
        self.accel_scale = 16384.0  # ±2g range
        self.last_speed = 0.0
        
        # 速度計算用変数
        self.velocity_x = 0.0
        self.velocity_y = 0.0
        self.velocity_z = 0.0
        self.last_time = time.time()
        
    def initialize(self) -> bool:
        """加速度センサーを初期化"""
        if self.mock_mode:
            logger.info("Speed sensor initialized in mock mode")
            return True
            
        try:
            self.bus = smbus2.SMBus(self.i2c_bus)
            
            # MPU6050を起動（スリープモード解除）
            self.bus.write_byte_data(self.device_address, 0x6B, 0)
            
            # 加速度センサー設定（±2g range）
            self.bus.write_byte_data(self.device_address, 0x1C, 0)
            
            time.sleep(0.1)
            
            # 初回読み取りテスト
            test_accel = self._read_accelerometer()
            logger.info(f"Speed sensor initialized successfully. Test reading: {test_accel}")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize speed sensor: {e}")
            logger.info("Falling back to mock mode")
            self.mock_mode = True
            return True  # モックモードで継続
    
    def _read_accelerometer(self) -> Tuple[float, float, float]:
        """生の加速度データを読み取り"""
        if self.mock_mode:
            return self._get_mock_acceleration()
        
        try:
            # 加速度データを読み取り（6バイト）
            data = self.bus.read_i2c_block_data(self.device_address, 0x3B, 6)
            
            # 16ビット符号付き整数に変換
            accel_x = self._convert_to_signed_16(data[0], data[1])
            accel_y = self._convert_to_signed_16(data[2], data[3])
            accel_z = self._convert_to_signed_16(data[4], data[5])
            
            # g単位に変換
            accel_x = accel_x / self.accel_scale
            accel_y = accel_y / self.accel_scale
            accel_z = accel_z / self.accel_scale
            
            return (accel_x, accel_y, accel_z)
            
        except Exception as e:
            logger.error(f"Failed to read accelerometer: {e}")
            return (0.0, 0.0, 0.0)
    
    def _convert_to_signed_16(self, high_byte: int, low_byte: int) -> int:
        """2バイトを16ビット符号付き整数に変換"""
        value = (high_byte << 8) + low_byte
        if value >= 32768:
            value = value - 65536
        return value
    
    def get_speed(self) -> float:
        """現在の速度を取得（km/h換算）"""
        accel_x, accel_y, accel_z = self._read_accelerometer()
        
        # 重力加速度を除去（簡易版）
        accel_magnitude = math.sqrt(accel_x**2 + accel_y**2 + accel_z**2)
        gravity = 1.0  # 1g
        
        # 重力を差し引いた実際の加速度
        net_acceleration = max(0, accel_magnitude - gravity)
        
        # 速度積分（簡易版）
        current_time = time.time()
        dt = current_time - self.last_time
        
        if dt > 0.001:  # 1ms以上の時間差がある場合
            # 速度更新（m/s）
            self.velocity_x += net_acceleration * dt
            
            # 速度の減衰（摩擦をシミュレート）
            decay_factor = 0.95
            self.velocity_x *= decay_factor
            
            # km/h に変換
            speed_kmh = abs(self.velocity_x) * 3.6
            
            self.last_speed = speed_kmh
            self.last_time = current_time
        
        return self.last_speed
    
    def _get_mock_acceleration(self) -> Tuple[float, float, float]:
        """モック用加速度データ生成"""
        # 時間に応じて変化するモックデータ
        t = time.time()
        
        # 車の動きをシミュレート
        accel_x = math.sin(t * 0.5) * 0.3  # 前後加速度
        accel_y = math.cos(t * 0.3) * 0.2  # 横加速度
        accel_z = 1.0 + math.sin(t * 0.1) * 0.1  # 重力 + 振動
        
        return (accel_x, accel_y, accel_z)
    
    def get_acceleration_vector(self) -> Tuple[float, float, float]:
        """3軸加速度ベクトルを取得"""
        return self._read_accelerometer()
    
    def is_available(self) -> bool:
        """センサーが利用可能かチェック"""
        if self.mock_mode:
            return True
        return self.bus is not None
    
    def get_sensor_info(self) -> dict:
        """センサー情報を取得"""
        if self.mock_mode:
            return {
                "type": "Mock",
                "address": "mock",
                "bus": "mock",
                "status": "active"
            }
        
        if self.bus:
            return {
                "type": "MPU6050",
                "address": hex(self.device_address),
                "bus": self.i2c_bus,
                "status": "active"
            }
        
        return {
            "type": "Unknown",
            "address": "none",
            "bus": "none",
            "status": "inactive"
        }
    
    def cleanup(self):
        """リソースクリーンアップ"""
        if self.bus:
            try:
                self.bus.close()
            except:
                pass