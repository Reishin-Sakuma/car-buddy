#!/usr/bin/env python3
"""
Car-Buddy UI表示テスト
"""

import sys
import os

# プロジェクトルートをパスに追加
sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

import tkinter as tk
import config
import logging

# ログ設定
logging.basicConfig(level=logging.INFO)
logger = logging.getLogger(__name__)

def test_basic_ui():
    """基本UI表示テスト"""
    print("🚗 Car-Buddy UI表示テスト")
    print("=" * 40)
    
    try:
        # メインウィンドウ作成
        root = tk.Tk()
        root.title("Car-Buddy UI Test")
        root.geometry("800x480")
        root.configure(bg='#1a1a2e')
        
        # タイトル
        title_label = tk.Label(
            root, 
            text="Car-Buddy", 
            font=('Arial', 24, 'bold'),
            fg='white',
            bg='#1a1a2e'
        )
        title_label.pack(pady=20)
        
        # 温度表示エリア
        temp_frame = tk.Frame(root, bg='#1a1a2e')
        temp_frame.pack(pady=10)
        
        temp_label = tk.Label(
            temp_frame,
            text="温度:",
            font=('Arial', 16),
            fg='white',
            bg='#1a1a2e'
        )
        temp_label.pack(side=tk.LEFT)
        
        temp_value = tk.Label(
            temp_frame,
            text="25.0°C",
            font=('Arial', 16, 'bold'),
            fg='#00ff88',
            bg='#1a1a2e'
        )
        temp_value.pack(side=tk.LEFT, padx=10)
        
        # 速度表示エリア
        speed_frame = tk.Frame(root, bg='#1a1a2e')
        speed_frame.pack(pady=10)
        
        speed_label = tk.Label(
            speed_frame,
            text="速度:",
            font=('Arial', 16),
            fg='white',
            bg='#1a1a2e'
        )
        speed_label.pack(side=tk.LEFT)
        
        speed_value = tk.Label(
            speed_frame,
            text="0 km/h",
            font=('Arial', 16, 'bold'),
            fg='#ff6b6b',
            bg='#1a1a2e'
        )
        speed_value.pack(side=tk.LEFT, padx=10)
        
        # 時刻表示
        time_label = tk.Label(
            root,
            text="2025-07-20 15:30:00",
            font=('Arial', 14),
            fg='#4ecdc4',
            bg='#1a1a2e'
        )
        time_label.pack(pady=20)
        
        # ステータス表示
        status_label = tk.Label(
            root,
            text="✅ UI表示テスト - 正常動作中",
            font=('Arial', 12),
            fg='#ffe66d',
            bg='#1a1a2e'
        )
        status_label.pack(pady=10)
        
        # 終了ボタン
        exit_button = tk.Button(
            root,
            text="テスト終了",
            font=('Arial', 12),
            command=root.quit,
            bg='#ff6b6b',
            fg='white',
            padx=20,
            pady=5
        )
        exit_button.pack(pady=20)
        
        print("✅ UI要素の作成完了")
        print("📺 ウィンドウを表示中...")
        print("💡 「テスト終了」ボタンまたはウィンドウを閉じて終了")
        
        # メインループ開始
        root.mainloop()
        
        print("✅ UI表示テスト完了")
        return True
        
    except Exception as e:
        print(f"❌ UI表示エラー: {e}")
        return False

if __name__ == "__main__":
    test_basic_ui()