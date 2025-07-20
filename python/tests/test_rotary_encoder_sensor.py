# CarBuddy Python版 - ロータリーエンコーダーセンサーテスト
import unittest
import sys
import os
import time
import threading

# プロジェクトルートをパスに追加
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..', '..'))

from python.sensors.rotary_encoder_sensor import RotaryEncoderSensor
import config

class TestRotaryEncoderSensor(unittest.TestCase):
    """ロータリーエンコーダーセンサーのテストクラス"""
    
    def setUp(self):
        """テスト前の準備"""
        # テスト用設定（モックモードを強制）
        self.test_config = {
            "MOCK_SENSORS": True,
            "ROTARY_ENCODER_CLK_PIN": config.ROTARY_ENCODER_CLK_PIN,
            "ROTARY_ENCODER_DT_PIN": config.ROTARY_ENCODER_DT_PIN
        }
        self.sensor = RotaryEncoderSensor(self.test_config)
    
    def tearDown(self):
        """テスト後のクリーンアップ"""
        self.sensor.cleanup()
    
    def test_initialization(self):
        """初期化テスト"""
        result = self.sensor.initialize()
        self.assertTrue(result)
        self.assertTrue(self.sensor.mock_mode)
        
        status = self.sensor.get_status()
        self.assertTrue(status["initialized"])
        self.assertTrue(status["mock_mode"])
        self.assertEqual(status["position"], 0)
    
    def test_position_management(self):
        """ポジション管理テスト"""
        self.sensor.initialize()
        
        # 初期位置
        self.assertEqual(self.sensor.get_position(), 0)
        
        # ポジション設定
        self.sensor.set_position(10)
        self.assertEqual(self.sensor.get_position(), 10)
        
        # ポジションリセット
        self.sensor.reset_position()
        self.assertEqual(self.sensor.get_position(), 0)
    
    def test_mock_rotation(self):
        """モック回転テスト"""
        self.sensor.initialize()
        
        # 時計回り回転
        self.sensor.mock_rotate_clockwise()
        self.assertEqual(self.sensor.get_position(), 1)
        
        self.sensor.mock_rotate_clockwise()
        self.assertEqual(self.sensor.get_position(), 2)
        
        # 反時計回り回転
        self.sensor.mock_rotate_counter_clockwise()
        self.assertEqual(self.sensor.get_position(), 1)
        
        self.sensor.mock_rotate_counter_clockwise()
        self.assertEqual(self.sensor.get_position(), 0)
    
    def test_relative_change(self):
        """相対変化量テスト"""
        self.sensor.initialize()
        
        # 初回は0
        change = self.sensor.get_relative_change()
        self.assertEqual(change, 0)
        
        # 2回転させて変化量確認
        self.sensor.mock_rotate_clockwise()
        self.sensor.mock_rotate_clockwise()
        change = self.sensor.get_relative_change()
        self.assertEqual(change, 2)
        
        # 次の取得では0（変化なし）
        change = self.sensor.get_relative_change()
        self.assertEqual(change, 0)
    
    def test_callbacks(self):
        """コールバック機能テスト"""
        self.sensor.initialize()
        
        # コールバック結果を記録
        self.callback_results = []
        
        def on_rotate(position):
            self.callback_results.append(f"rotate:{position}")
        
        def on_clockwise():
            self.callback_results.append("clockwise")
        
        def on_counter_clockwise():
            self.callback_results.append("counter_clockwise")
        
        # コールバック設定
        self.sensor.set_on_rotate_callback(on_rotate)
        self.sensor.set_on_clockwise_callback(on_clockwise)
        self.sensor.set_on_counter_clockwise_callback(on_counter_clockwise)
        
        # 回転実行
        self.sensor.mock_rotate_clockwise()
        self.assertIn("clockwise", self.callback_results)
        self.assertIn("rotate:1", self.callback_results)
        
        self.callback_results.clear()
        
        self.sensor.mock_rotate_counter_clockwise()
        self.assertIn("counter_clockwise", self.callback_results)
        self.assertIn("rotate:0", self.callback_results)
    
    def test_thread_safety(self):
        """スレッドセーフティテスト"""
        self.sensor.initialize()
        
        def rotate_worker():
            for _ in range(10):
                self.sensor.mock_rotate_clockwise()
                time.sleep(0.001)
        
        # 複数スレッドで同時実行
        threads = []
        for _ in range(3):
            thread = threading.Thread(target=rotate_worker)
            threads.append(thread)
            thread.start()
        
        for thread in threads:
            thread.join()
        
        # 30回転の結果確認
        self.assertEqual(self.sensor.get_position(), 30)
    
    def test_status_information(self):
        """ステータス情報テスト"""
        self.sensor.initialize()
        
        status = self.sensor.get_status()
        
        # 必要なキーが含まれているか確認
        required_keys = ["mock_mode", "position", "clk_pin", "dt_pin", "initialized"]
        for key in required_keys:
            self.assertIn(key, status)
        
        # 値の確認
        self.assertTrue(status["mock_mode"])
        self.assertEqual(status["clk_pin"], config.ROTARY_ENCODER_CLK_PIN)
        self.assertEqual(status["dt_pin"], config.ROTARY_ENCODER_DT_PIN)
        self.assertTrue(status["initialized"])

class TestRotaryEncoderSensorManual(unittest.TestCase):
    """手動テスト用クラス（実際のハードウェアが必要）"""
    
    def setUp(self):
        """テスト前の準備"""
        # 実際のハードウェア用設定
        self.test_config = {
            "MOCK_SENSORS": False,
            "ROTARY_ENCODER_CLK_PIN": config.ROTARY_ENCODER_CLK_PIN,
            "ROTARY_ENCODER_DT_PIN": config.ROTARY_ENCODER_DT_PIN
        }
        self.sensor = RotaryEncoderSensor(self.test_config)
    
    def tearDown(self):
        """テスト後のクリーンアップ"""
        self.sensor.cleanup()
    
    @unittest.skip("Manual test - requires physical hardware")
    def test_real_hardware(self):
        """実ハードウェアテスト（手動実行用）"""
        result = self.sensor.initialize()
        self.assertTrue(result)
        
        print("\n" + "="*50)
        print("ロータリーエンコーダー手動テスト")
        print("エンコーダーを回転させてください（10秒間）")
        print("="*50)
        
        start_time = time.time()
        while time.time() - start_time < 10:
            position = self.sensor.get_position()
            print(f"\r現在位置: {position:3d}", end="", flush=True)
            time.sleep(0.1)
        
        final_position = self.sensor.get_position()
        print(f"\n最終位置: {final_position}")
        print("テスト完了")

if __name__ == "__main__":
    # 基本テストを実行
    unittest.main(argv=[''], exit=False, verbosity=2)
    
    # 手動テストを実行したい場合はコメントアウト
    # suite = unittest.TestLoader().loadTestsFromTestCase(TestRotaryEncoderSensorManual)
    # unittest.TextTestRunner(verbosity=2).run(suite)