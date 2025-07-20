#!/usr/bin/env python3
"""
DS18B20接続確認用シンプルスクリプト
接続とライブラリの基本動作のみをチェック
"""

import time
from pathlib import Path

def check_w1_devices():
    """1-Wireデバイスの基本確認"""
    print("🔍 DS18B20接続確認")
    print("-" * 30)
    
    w1_path = Path("/sys/bus/w1/devices")
    
    if not w1_path.exists():
        print("❌ 1-Wireが有効化されていません")
        print("解決方法:")
        print("1. sudo raspi-config")
        print("2. Interface Options → 1-Wire → Enable")
        print("3. 再起動")
        return False
    
    print("✅ 1-Wireが有効化されています")
    
    # DS18B20デバイス検索
    ds18b20_devices = []
    for device in w1_path.iterdir():
        if device.name.startswith('28-'):
            ds18b20_devices.append(device.name)
    
    if ds18b20_devices:
        print(f"✅ DS18B20デバイス発見: {len(ds18b20_devices)}個")
        for device in ds18b20_devices:
            print(f"   📍 {device}")
        return True
    else:
        print("❌ DS18B20デバイスが見つかりません")
        print("配線確認:")
        print("   📍 GPIO4 ── DS18B20 DATA")
        print("   📍 3.3V ── DS18B20 VCC")
        print("   📍 GND ── DS18B20 GND")
        print("   📍 4.7kΩ抵抗: 3.3V ─┬─ DATA")
        return False

def test_temperature_reading():
    """温度読み取りテスト"""
    print("\n🌡️ 温度読み取りテスト")
    print("-" * 30)
    
    try:
        from w1thermsensor import W1ThermSensor
        print("✅ w1thermsensorライブラリOK")
        
        sensor = W1ThermSensor()
        print(f"✅ センサー接続OK: {sensor.id}")
        
        print("温度測定中...")
        for i in range(5):
            temp = sensor.get_temperature()
            print(f"   {i+1}回目: {temp:.1f}°C")
            time.sleep(1)
        
        return True
        
    except ImportError:
        print("❌ w1thermsensorライブラリなし")
        print("インストール: pip install w1thermsensor")
        return False
    except Exception as e:
        print(f"❌ エラー: {e}")
        return False

def main():
    """メイン実行"""
    print("🚗 Car-Buddy DS18B20確認スクリプト")
    print("=" * 40)
    
    # 接続確認
    if check_w1_devices():
        # 温度読み取りテスト
        test_temperature_reading()
    
    print("\n" + "=" * 40)
    print("確認完了")

if __name__ == "__main__":
    main()