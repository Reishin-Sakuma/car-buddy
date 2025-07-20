#!/usr/bin/env python3
"""
Car-Buddy UI基本テスト（ファイル出力版）
"""

import sys
import os

# プロジェクトルートをパスに追加
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

try:
    import tkinter as tk
    print("✅ tkinter import 成功")
except Exception as e:
    print(f"❌ tkinter import エラー: {e}")
    sys.exit(1)

try:
    import config
    print("✅ config import 成功")
except Exception as e:
    print(f"❌ config import エラー: {e}")
    sys.exit(1)

def test_ui_components():
    """UI要素の作成テスト"""
    try:
        # ルートウィンドウ作成（表示はしない）
        root = tk.Tk()
        root.withdraw()  # ウィンドウを隠す
        
        print("✅ ルートウィンドウ作成成功")
        
        # 基本要素のテスト
        label = tk.Label(root, text="Test Label")
        print("✅ Labelウィジェット作成成功")
        
        button = tk.Button(root, text="Test Button")
        print("✅ Buttonウィジェット作成成功")
        
        frame = tk.Frame(root)
        print("✅ Frameウィジェット作成成功")
        
        # 設定値の確認
        print(f"✅ 画面サイズ設定: {config.SCREEN_WIDTH}x{config.SCREEN_HEIGHT}")
        print(f"✅ フルスクリーン設定: {config.FULLSCREEN}")
        print(f"✅ モックセンサー設定: {config.MOCK_SENSORS}")
        
        # クリーンアップ
        root.destroy()
        print("✅ UI基本機能テスト完了")
        
        return True
        
    except Exception as e:
        print(f"❌ UI要素作成エラー: {e}")
        return False

def test_display_manager_import():
    """DisplayManagerのインポートテスト"""
    try:
        from python.ui.display_manager import DisplayManager
        print("✅ DisplayManager import 成功")
        
        # 設定作成
        test_config = {
            "SCREEN_WIDTH": config.SCREEN_WIDTH,
            "SCREEN_HEIGHT": config.SCREEN_HEIGHT,
            "FULLSCREEN": False,  # テスト用にフルスクリーン無効
            "CHARACTER_IMAGE_DIR": config.CHARACTER_IMAGE_DIR,
            "CHARACTER_IMAGE_FILES": config.CHARACTER_IMAGE_FILES,
            "COLORS": config.COLORS,
            "TEMP_THRESHOLDS": config.TEMP_THRESHOLDS,
        }
        
        # DisplayManager作成（初期化はしない）
        display_manager = DisplayManager(test_config)
        print("✅ DisplayManager インスタンス作成成功")
        
        return True
        
    except Exception as e:
        print(f"❌ DisplayManager import/作成エラー: {e}")
        return False

if __name__ == "__main__":
    print("🚗 Car-Buddy UI基本テスト")
    print("=" * 40)
    
    success = True
    
    # 基本UIテスト
    if not test_ui_components():
        success = False
    
    print("-" * 40)
    
    # DisplayManagerテスト
    if not test_display_manager_import():
        success = False
    
    print("=" * 40)
    
    if success:
        print("✅ 全てのUIテストが成功しました")
        print("📺 実際のUI表示にはディスプレイ環境が必要です")
        print("💡 Raspberry Piに接続されたモニターで実行してください")
    else:
        print("❌ いくつかのテストが失敗しました")
    
    print("テスト完了")