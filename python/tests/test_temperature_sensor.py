# CarBuddy Python版 - 温度センサーテスト
import pytest
import sys
import os

# テスト対象モジュールをインポート
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from sensors.temperature_sensor import TemperatureSensor

class TestTemperatureSensor:
    """温度センサーのテストクラス"""
    
    @pytest.fixture
    def mock_config(self):
        """テスト用設定"""
        return {
            "MOCK_SENSORS": True,
            "TEMP_SENSOR_ID": None
        }
    
    @pytest.fixture
    def sensor(self, mock_config):
        """テスト用センサーインスタンス"""
        return TemperatureSensor(mock_config)
    
    def test_sensor_initialization(self, sensor):
        """センサー初期化テスト"""
        # When: センサーを初期化
        result = sensor.initialize()
        
        # Then: 初期化が成功する
        assert result == True
        assert sensor.is_available() == True
    
    def test_temperature_reading_range(self, sensor):
        """温度読み取り範囲テスト"""
        # Given: 初期化されたセンサー
        sensor.initialize()
        
        # When: 温度を読み取り
        temperature = sensor.get_temperature()
        
        # Then: 妥当な温度範囲の値が返される
        assert isinstance(temperature, float)
        assert -40.0 <= temperature <= 85.0  # DS18B20の測定範囲
    
    def test_temperature_reading_consistency(self, sensor):
        """温度読み取り一貫性テスト"""
        # Given: 初期化されたセンサー
        sensor.initialize()
        
        # When: 複数回温度を読み取り
        temperatures = [sensor.get_temperature() for _ in range(5)]
        
        # Then: すべて数値で、極端な変化がない
        for temp in temperatures:
            assert isinstance(temp, float)
        
        # 連続読み取りの差が20度以内（モックでの妥当な範囲）
        max_diff = max(temperatures) - min(temperatures)
        assert max_diff <= 20.0
    
    def test_sensor_info(self, sensor):
        """センサー情報テスト"""
        # Given: 初期化されたセンサー
        sensor.initialize()
        
        # When: センサー情報を取得
        info = sensor.get_sensor_info()
        
        # Then: 必要な情報が含まれている
        assert "type" in info
        assert "id" in info
        assert "status" in info
        assert info["type"] == "Mock"  # モックモードのため
        assert info["status"] == "active"
    
    def test_mock_temperature_variation(self, sensor):
        """モック温度変動テスト"""
        # Given: モックモードのセンサー
        sensor.initialize()
        
        # When: 時間をずらして複数回読み取り
        import time
        temp1 = sensor.get_temperature()
        time.sleep(0.1)
        temp2 = sensor.get_temperature()
        
        # Then: 値が変化する可能性がある（モックは時間ベース）
        # 注意: 値が同じ場合もあるため、型と範囲のみチェック
        assert isinstance(temp1, float)
        assert isinstance(temp2, float)