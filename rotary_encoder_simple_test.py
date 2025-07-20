#!/usr/bin/env python3
"""
簡単なロータリーエンコーダーテスト
"""

import time
from gpiozero import RotaryEncoder

# ピン設定
CLK_PIN = 17
DT_PIN = 18

print("🚗 ロータリーエンコーダー簡単テスト")
print("=" * 40)

try:
    # エンコーダー初期化
    encoder = RotaryEncoder(CLK_PIN, DT_PIN)
    print(f"✅ エンコーダー初期化完了 (CLK: GPIO{CLK_PIN}, DT: GPIO{DT_PIN})")
    
    position = 0
    
    def clockwise():
        global position
        position += 1
        print(f"→ 時計回り: {position}")
    
    def counter_clockwise():
        global position
        position -= 1
        print(f"← 反時計回り: {position}")
    
    encoder.when_rotated_clockwise = clockwise
    encoder.when_rotated_counter_clockwise = counter_clockwise
    
    print("📊 10秒間監視します... エンコーダーを回してください")
    
    for i in range(10):
        print(f"  {i+1}秒: 現在位置 = {position}")
        time.sleep(1)
    
    print(f"\n結果: 最終位置 = {position}")
    
except Exception as e:
    print(f"❌ エラー: {e}")
    
finally:
    try:
        encoder.close()
        print("🔧 リソース解放完了")
    except:
        pass

print("テスト完了")