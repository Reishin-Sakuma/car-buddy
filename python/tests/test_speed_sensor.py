# CarBuddy Python版 - 速度センサーテスト
import pytest
import sys
import os

# テスト対象モジュールをインポート
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from sensors.speed_sensor import SpeedSensor

class TestSpeedSensor:
    """速度センサーのテストクラス"""
    
    @pytest.fixture
    def mock_config(self):
        """テスト用設定"""
        return {
            "MOCK_SENSORS": True,
            "MPU6050_I2C_BUS": 1,
            "MPU6050_ADDRESS": 0x68
        }
    
    @pytest.fixture
    def sensor(self, mock_config):
        """テスト用センサーインスタンス"""
        return SpeedSensor(mock_config)
    
    def test_sensor_initialization(self, sensor):
        """センサー初期化テスト"""
        # When: センサーを初期化
        result = sensor.initialize()
        
        # Then: 初期化が成功する
        assert result == True
        assert sensor.is_available() == True
    
    def test_speed_reading_type(self, sensor):
        """速度読み取り型テスト"""
        # Given: 初期化されたセンサー
        sensor.initialize()
        
        # When: 速度を読み取り
        speed = sensor.get_speed()
        
        # Then: 数値型で非負の値が返される
        assert isinstance(speed, float)
        assert speed >= 0.0
    
    def test_acceleration_vector_reading(self, sensor):
        """加速度ベクトル読み取りテスト"""
        # Given: 初期化されたセンサー
        sensor.initialize()
        
        # When: 加速度ベクトルを読み取り
        accel_x, accel_y, accel_z = sensor.get_acceleration_vector()
        
        # Then: 3つの数値が返される
        assert isinstance(accel_x, float)
        assert isinstance(accel_y, float)
        assert isinstance(accel_z, float)
        
        # 妥当な加速度範囲（±2g設定）
        for accel in [accel_x, accel_y, accel_z]:
            assert -2.5 <= accel <= 2.5
    
    def test_speed_calculation_stability(self, sensor):
        """速度計算安定性テスト"""
        # Given: 初期化されたセンサー
        sensor.initialize()
        
        # When: 複数回速度を読み取り
        speeds = []
        for _ in range(10):
            speed = sensor.get_speed()
            speeds.append(speed)
            import time
            time.sleep(0.01)  # 10ms間隔
        
        # Then: すべて数値で、異常値がない
        for speed in speeds:
            assert isinstance(speed, float)
            assert 0.0 <= speed <= 200.0  # 妥当な速度範囲
    
    def test_sensor_info(self, sensor):
        """センサー情報テスト"""
        # Given: 初期化されたセンサー
        sensor.initialize()
        
        # When: センサー情報を取得
        info = sensor.get_sensor_info()
        
        # Then: 必要な情報が含まれている
        assert "type" in info
        assert "address" in info
        assert "bus" in info
        assert "status" in info
        assert info["type"] == "Mock"  # モックモードのため
        assert info["status"] == "active"
    
    def test_cleanup_method(self, sensor):
        """クリーンアップメソッドテスト"""
        # Given: 初期化されたセンサー
        sensor.initialize()
        
        # When: クリーンアップを実行
        # Then: エラーが発生しない
        sensor.cleanup()  # 例外が発生しなければOK