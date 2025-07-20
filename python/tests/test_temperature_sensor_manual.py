#!/usr/bin/env python3
"""
TemperatureSensorクラスの手動テスト
実際のDS18B20センサーでの動作確認用
"""

import sys
import time
import logging
from pathlib import Path

# プロジェクトルートを追加
project_root = Path(__file__).parent.parent.parent
sys.path.insert(0, str(project_root))

import config
from python.sensors.temperature_sensor import TemperatureSensor

# ログ設定
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def test_sensor_initialization():
    """センサー初期化テスト"""
    print("🔧 センサー初期化テスト")
    print("-" * 30)
    
    sensor = TemperatureSensor(config.__dict__)
    
    if sensor.initialize():
        print("✅ 初期化成功")
        return sensor
    else:
        print("❌ 初期化失敗")
        return None

def test_sensor_info(sensor):
    """センサー情報取得テスト"""
    print("\n📋 センサー情報テスト")
    print("-" * 30)
    
    info = sensor.get_sensor_info()
    print(f"タイプ: {info['type']}")
    print(f"ID: {info['id']}")
    print(f"ステータス: {info['status']}")
    
    available = sensor.is_available()
    print(f"利用可能: {'はい' if available else 'いいえ'}")

def test_temperature_reading(sensor):
    """温度読み取りテスト"""
    print("\n🌡️ 温度読み取りテスト")
    print("-" * 30)
    
    temperatures = []
    
    print("10回連続測定:")
    for i in range(10):
        temp = sensor.get_temperature()
        temperatures.append(temp)
        print(f"  {i+1:2d}: {temp:5.2f}°C")
        time.sleep(0.5)
    
    # 統計情報
    if temperatures:
        avg = sum(temperatures) / len(temperatures)
        min_temp = min(temperatures)
        max_temp = max(temperatures)
        variation = max_temp - min_temp
        
        print(f"\n📊 統計情報:")
        print(f"  平均: {avg:.2f}°C")
        print(f"  最小: {min_temp:.2f}°C")
        print(f"  最大: {max_temp:.2f}°C")
        print(f"  変動: {variation:.2f}°C")
        
        # 温度判定テスト
        threshold = config.TEMP_THRESHOLDS
        print(f"\n🚨 閾値判定:")
        print(f"  現在温度: {temperatures[-1]:.1f}°C")
        print(f"  警告温度: {threshold['hot_warning']:.1f}°C")
        print(f"  変化閾値: {threshold['transition']:.1f}°C")
        
        if temperatures[-1] >= threshold['hot_warning']:
            print("  ⚠️ 高温警告!")
        elif temperatures[-1] >= threshold['transition']:
            print("  🟡 注意レベル")
        else:
            print("  ✅ 正常範囲")

def test_error_handling(sensor):
    """エラーハンドリングテスト"""
    print("\n🛡️ エラーハンドリングテスト")
    print("-" * 30)
    
    # 複数回連続読み取り（センサー負荷テスト）
    print("高速連続読み取り（50ms間隔）:")
    for i in range(20):
        temp = sensor.get_temperature()
        print(f"  {i+1:2d}: {temp:5.2f}°C", end=" " if i % 5 != 4 else "\n")
        time.sleep(0.05)
    
    print("\n✅ エラーハンドリングテスト完了")

def test_mock_mode():
    """モックモードテスト"""
    print("\n🎭 モックモードテスト")
    print("-" * 30)
    
    # モックモード設定
    mock_config = config.__dict__.copy()
    mock_config['MOCK_SENSORS'] = True
    
    mock_sensor = TemperatureSensor(mock_config)
    mock_sensor.initialize()
    
    print("モック温度データ（5回測定）:")
    for i in range(5):
        temp = mock_sensor.get_temperature()
        print(f"  {i+1}: {temp:.2f}°C")
        time.sleep(1)

def main():
    """メインテスト実行"""
    print("🚗 Car-Buddy TemperatureSensor手動テスト")
    print("=" * 50)
    
    # センサー初期化
    sensor = test_sensor_initialization()
    
    if sensor:
        # センサー情報テスト
        test_sensor_info(sensor)
        
        # 温度読み取りテスト
        test_temperature_reading(sensor)
        
        # エラーハンドリングテスト
        test_error_handling(sensor)
    
    # モックモードテスト
    test_mock_mode()
    
    print("\n" + "=" * 50)
    print("🌡️ 温度センサーテスト完了")

if __name__ == "__main__":
    main()