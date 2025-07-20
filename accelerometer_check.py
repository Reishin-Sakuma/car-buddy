#!/usr/bin/env python3
"""
Car-Buddy 加速度センサー確認スクリプト
MPU6050加速度センサーの接続と動作を確認
"""

import smbus
import time
import sys

# MPU6050のI2Cアドレス
MPU6050_ADDR = 0x68

# MPU6050レジスタアドレス
PWR_MGMT_1 = 0x6B
ACCEL_XOUT_H = 0x3B
ACCEL_YOUT_H = 0x3D
ACCEL_ZOUT_H = 0x3F

def check_i2c_connection():
    """I2C接続の確認"""
    try:
        bus = smbus.SMBus(1)
        devices = []
        
        print("🔍 I2Cデバイススキャン中...")
        for addr in range(0x03, 0x78):
            try:
                bus.read_byte(addr)
                devices.append(hex(addr))
            except:
                pass
        
        if devices:
            print(f"✅ 検出されたI2Cデバイス: {', '.join(devices)}")
        else:
            print("❌ I2Cデバイスが見つかりません")
            
        return devices
    except Exception as e:
        print(f"❌ I2C接続エラー: {e}")
        return []

def init_mpu6050(bus):
    """MPU6050の初期化"""
    try:
        # スリープモードを解除
        bus.write_byte_data(MPU6050_ADDR, PWR_MGMT_1, 0)
        time.sleep(0.1)
        print("✅ MPU6050を初期化しました")
        return True
    except Exception as e:
        print(f"❌ MPU6050初期化エラー: {e}")
        return False

def read_accelerometer(bus):
    """加速度データの読み取り"""
    try:
        # 加速度データを読み取り（16bit符号付き）
        accel_x = bus.read_word_data(MPU6050_ADDR, ACCEL_XOUT_H)
        accel_y = bus.read_word_data(MPU6050_ADDR, ACCEL_YOUT_H)
        accel_z = bus.read_word_data(MPU6050_ADDR, ACCEL_ZOUT_H)
        
        # ビッグエンディアン to リトルエンディアン変換
        accel_x = ((accel_x & 0xFF) << 8) | ((accel_x >> 8) & 0xFF)
        accel_y = ((accel_y & 0xFF) << 8) | ((accel_y >> 8) & 0xFF)
        accel_z = ((accel_z & 0xFF) << 8) | ((accel_z >> 8) & 0xFF)
        
        # 16bit符号付き整数に変換
        if accel_x > 32767:
            accel_x -= 65536
        if accel_y > 32767:
            accel_y -= 65536
        if accel_z > 32767:
            accel_z -= 65536
            
        # 実際のg値に変換（±2g設定での変換係数）
        accel_x_g = accel_x / 16384.0
        accel_y_g = accel_y / 16384.0
        accel_z_g = accel_z / 16384.0
        
        return accel_x_g, accel_y_g, accel_z_g
    except Exception as e:
        print(f"❌ 加速度データ読み取りエラー: {e}")
        return None, None, None

def main():
    print("=" * 40)
    print("🚗 Car-Buddy 加速度センサー確認スクリプト")
    print("=" * 40)
    
    # I2C接続確認
    print("🔧 I2C接続確認")
    print("-" * 30)
    devices = check_i2c_connection()
    
    if hex(MPU6050_ADDR) not in devices:
        print(f"❌ MPU6050 (アドレス: {hex(MPU6050_ADDR)}) が見つかりません")
        print("\n配線確認:")
        print("   📌 GPIO2 (SDA) ← MPU6050 SDA")
        print("   📌 GPIO3 (SCL) ← MPU6050 SCL")
        print("   📌 3.3V ← MPU6050 VCC")
        print("   📌 GND ← MPU6050 GND")
        print("\n=" * 40)
        print("確認完了")
        return False
    
    # MPU6050動作確認
    print("\n🎯 MPU6050動作確認")
    print("-" * 30)
    
    try:
        bus = smbus.SMBus(1)
        
        if not init_mpu6050(bus):
            return False
            
        print("📊 加速度データ取得中 (5秒間)...")
        for i in range(5):
            accel_x, accel_y, accel_z = read_accelerometer(bus)
            if accel_x is not None:
                print(f"   {i+1}秒: X={accel_x:6.2f}g, Y={accel_y:6.2f}g, Z={accel_z:6.2f}g")
            time.sleep(1)
            
        print("✅ MPU6050が正常に動作しています")
        
    except Exception as e:
        print(f"❌ MPU6050通信エラー: {e}")
        return False
    
    print("\n=" * 40)
    print("確認完了")
    return True

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\n⚠️  ユーザーによって中断されました")
        sys.exit(0)
    except Exception as e:
        print(f"\n❌ 予期しないエラー: {e}")
        sys.exit(1)