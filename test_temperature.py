#!/usr/bin/env python3
"""
DS18B20温度センサー動作確認スクリプト
Raspberry Pi Zero 2 W用
"""

import sys
import time
import logging
from pathlib import Path

# プロジェクトのpythonディレクトリをパスに追加
project_root = Path(__file__).parent
python_dir = project_root / "python"
sys.path.insert(0, str(python_dir))

# ログ設定
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s - %(levelname)s - %(message)s'
)
logger = logging.getLogger(__name__)

def check_w1_kernel_modules():
    """1-Wireカーネルモジュールの確認"""
    logger.info("=== 1-Wireカーネルモジュール確認 ===")
    
    try:
        # /proc/modulesでモジュール確認
        with open('/proc/modules', 'r') as f:
            modules = f.read()
            
        w1_modules = ['w1_gpio', 'w1_therm']
        for module in w1_modules:
            if module in modules:
                logger.info(f"✓ {module} モジュールがロードされています")
            else:
                logger.warning(f"✗ {module} モジュールがロードされていません")
                logger.info(f"  手動ロード: sudo modprobe {module}")
                
    except FileNotFoundError:
        logger.error("/proc/modules が見つかりません（非Linux環境?）")
        return False
    
    return True

def check_w1_devices():
    """1-Wireデバイスの確認"""
    logger.info("\n=== 1-Wireデバイス確認 ===")
    
    w1_path = Path("/sys/bus/w1/devices")
    
    if not w1_path.exists():
        logger.error("1-Wireデバイスパスが存在しません")
        logger.info("1-Wireを有効化してください: sudo raspi-config")
        return []
    
    devices = []
    for device_path in w1_path.iterdir():
        if device_path.name.startswith('28-'):  # DS18B20のファミリーコード
            devices.append(device_path.name)
            logger.info(f"✓ DS18B20デバイス発見: {device_path.name}")
            
            # デバイス詳細確認
            slave_file = device_path / "w1_slave"
            if slave_file.exists():
                try:
                    with open(slave_file, 'r') as f:
                        content = f.read()
                    logger.info(f"  デバイス状態: {content.strip()}")
                except Exception as e:
                    logger.error(f"  デバイス読み取りエラー: {e}")
    
    if not devices:
        logger.warning("DS18B20デバイスが見つかりません")
        logger.info("接続確認:")
        logger.info("  - GPIO4にDS18B20のDATAピンが接続されているか")
        logger.info("  - 4.7kΩプルアップ抵抗が3.3VとDATAピン間にあるか")
        logger.info("  - 電源とGNDが正しく接続されているか")
    
    return devices

def test_w1thermsensor_library():
    """w1thermsensorライブラリのテスト"""
    logger.info("\n=== w1thermsensorライブラリテスト ===")
    
    try:
        from w1thermsensor import W1ThermSensor, NoSensorFoundError
        logger.info("✓ w1thermsensorライブラリのインポート成功")
        
        # センサー検索
        try:
            sensors = W1ThermSensor.get_available_sensors()
            logger.info(f"✓ 利用可能なセンサー数: {len(sensors)}")
            
            for i, sensor in enumerate(sensors):
                logger.info(f"  センサー{i+1}: ID={sensor.id}, Type={sensor.type}")
                
            return sensors
            
        except NoSensorFoundError:
            logger.warning("✗ センサーが見つかりません")
            return []
            
    except ImportError as e:
        logger.error(f"✗ w1thermsensorライブラリのインポート失敗: {e}")
        logger.info("インストール: pip install w1thermsensor")
        return None

def test_temperature_reading():
    """温度読み取りテスト"""
    logger.info("\n=== 温度読み取りテスト ===")
    
    try:
        from w1thermsensor import W1ThermSensor
        
        # 最初のセンサーを使用
        sensor = W1ThermSensor()
        logger.info(f"使用センサー: ID={sensor.id}")
        
        # 10回読み取りテスト
        logger.info("10回の連続読み取りテスト:")
        temperatures = []
        
        for i in range(10):
            try:
                temp = sensor.get_temperature()
                temperatures.append(temp)
                logger.info(f"  読み取り{i+1}: {temp:.2f}°C")
                time.sleep(1)
                
            except Exception as e:
                logger.error(f"  読み取り{i+1} エラー: {e}")
        
        if temperatures:
            avg_temp = sum(temperatures) / len(temperatures)
            min_temp = min(temperatures)
            max_temp = max(temperatures)
            
            logger.info(f"\n温度統計:")
            logger.info(f"  平均: {avg_temp:.2f}°C")
            logger.info(f"  最小: {min_temp:.2f}°C")
            logger.info(f"  最大: {max_temp:.2f}°C")
            logger.info(f"  変動幅: {max_temp - min_temp:.2f}°C")
            
            return True
        
    except Exception as e:
        logger.error(f"温度読み取りテスト失敗: {e}")
        return False

def test_temperature_sensor_class():
    """TemperatureSensorクラスのテスト"""
    logger.info("\n=== TemperatureSensorクラステスト ===")
    
    try:
        # configをインポート
        import config
        from sensors.temperature_sensor import TemperatureSensor
        
        # センサーインスタンス作成
        temp_sensor = TemperatureSensor(config.__dict__)
        
        # 初期化
        if temp_sensor.initialize():
            logger.info("✓ TemperatureSensor初期化成功")
            
            # センサー情報取得
            sensor_info = temp_sensor.get_sensor_info()
            logger.info(f"センサー情報: {sensor_info}")
            
            # 温度読み取りテスト
            logger.info("5回の温度読み取りテスト:")
            for i in range(5):
                temp = temp_sensor.get_temperature()
                logger.info(f"  読み取り{i+1}: {temp:.2f}°C")
                time.sleep(1)
            
            return True
        else:
            logger.error("✗ TemperatureSensor初期化失敗")
            return False
            
    except Exception as e:
        logger.error(f"TemperatureSensorクラステスト失敗: {e}")
        return False

def main():
    """メイン実行関数"""
    logger.info("🌡️ DS18B20温度センサー動作確認スタート")
    logger.info("=" * 50)
    
    # カーネルモジュール確認
    check_w1_kernel_modules()
    
    # デバイス確認
    devices = check_w1_devices()
    
    # ライブラリテスト
    sensors = test_w1thermsensor_library()
    
    # 温度読み取りテスト
    if sensors:
        test_temperature_reading()
    
    # プロジェクトのTemperatureSensorクラステスト
    test_temperature_sensor_class()
    
    logger.info("\n" + "=" * 50)
    logger.info("🌡️ 温度センサー動作確認完了")

if __name__ == "__main__":
    main()