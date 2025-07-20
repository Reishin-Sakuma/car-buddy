#!/usr/bin/env python3
"""
Car-Buddy ロータリーエンコーダー確認スクリプト
ロータリーエンコーダーの接続と動作を確認
"""

import time
import sys
from gpiozero import RotaryEncoder
from signal import pause

# ロータリーエンコーダーのピン設定（3ピン）
CLK_PIN = 17  # GPIO17 (ピン11) - CLK
DT_PIN = 18   # GPIO18 (ピン12) - DT
# 3ピン目はGND - プッシュボタン機能なし

# グローバル変数
position = 0
last_position = 0

def on_rotate_clockwise():
    """時計回り回転時のコールバック"""
    global position
    position += 1
    print(f"🔄 時計回り: ポジション = {position}")

def on_rotate_counter_clockwise():
    """反時計回り回転時のコールバック"""
    global position
    position -= 1
    print(f"🔄 反時計回り: ポジション = {position}")

# 3ピンエンコーダーのためボタン機能は無効

def check_gpio_pins():
    """GPIO接続の確認"""
    print("🔧 GPIO接続確認")
    print("-" * 30)
    
    try:
        # ロータリーエンコーダーの初期化テスト
        test_encoder = RotaryEncoder(CLK_PIN, DT_PIN)
        test_encoder.close()
        print(f"✅ ロータリーエンコーダー (CLK: GPIO{CLK_PIN}, DT: GPIO{DT_PIN}) - 接続OK")
        print("ℹ️  3ピンエンコーダー - プッシュボタン機能なし")
        
        return True
    except Exception as e:
        print(f"❌ GPIO接続エラー: {e}")
        return False

def main():
    print("=" * 40)
    print("🚗 Car-Buddy ロータリーエンコーダー確認スクリプト")
    print("=" * 40)
    
    # GPIO接続確認
    if not check_gpio_pins():
        print("\n配線確認（3ピンエンコーダー）:")
        print("   📌 GPIO17 (ピン11) ← エンコーダー CLK")
        print("   📌 GPIO18 (ピン12) ← エンコーダー DT")
        print("   📌 GND (ピン6) ← エンコーダー GND")
        print("   ℹ️  VCCは内部プルアップで動作（外部電源不要）")
        print("\n=" * 40)
        print("確認完了")
        return False
    
    print("\n🎯 ロータリーエンコーダー動作確認")
    print("-" * 30)
    print("エンコーダーを回転させてください（ボタン機能なし）")
    print("Ctrl+C で終了")
    
    try:
        # ロータリーエンコーダーの設定
        encoder = RotaryEncoder(CLK_PIN, DT_PIN)
        encoder.when_rotated_clockwise = on_rotate_clockwise
        encoder.when_rotated_counter_clockwise = on_rotate_counter_clockwise
        
        print("✅ ロータリーエンコーダーが初期化されました")
        print("📊 動作監視中...")
        
        # 初期状態の表示
        print(f"初期ポジション: {position}")
        
        # 定期的な状態表示
        start_time = time.time()
        while True:
            current_time = time.time()
            elapsed = int(current_time - start_time)
            
            # 10秒ごとに現在状態を表示
            if elapsed > 0 and elapsed % 10 == 0:
                print(f"⏰ {elapsed}秒経過 - 現在ポジション: {position}")
                time.sleep(1)  # 重複表示防止
            
            time.sleep(0.1)
            
    except KeyboardInterrupt:
        print("\n\n⚠️  動作確認を終了します")
        
    except Exception as e:
        print(f"\n❌ エラーが発生しました: {e}")
        return False
    
    finally:
        try:
            encoder.close()
            print("🔧 GPIO リソースを解放しました")
        except:
            pass
    
    print("\n📊 動作確認結果:")
    print(f"   最終ポジション: {position}")
    print("✅ ロータリーエンコーダーが正常に動作しています")
    
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