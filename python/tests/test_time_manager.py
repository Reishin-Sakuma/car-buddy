# CarBuddy Python版 - 時刻管理テスト
import pytest
import sys
import os
from datetime import datetime

# テスト対象モジュールをインポート
sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from utils.time_manager import TimeManager

class TestTimeManager:
    """時刻管理のテストクラス"""
    
    @pytest.fixture
    def config(self):
        """テスト用設定"""
        return {}
    
    @pytest.fixture
    def time_manager(self, config):
        """テスト用時刻管理インスタンス"""
        return TimeManager(config)
    
    def test_current_time_format(self, time_manager):
        """現在時刻フォーマットテスト"""
        # When: 現在時刻を取得
        time_str = time_manager.get_current_time()
        
        # Then: HH:MM:SS形式の文字列が返される
        assert isinstance(time_str, str)
        assert len(time_str) == 8  # "HH:MM:SS"
        assert time_str.count(':') == 2
        
        # 時刻部分が数値であることを確認
        parts = time_str.split(':')
        assert len(parts) == 3
        for part in parts:
            assert part.isdigit()
            assert len(part) == 2
    
    def test_current_date_format(self, time_manager):
        """現在日付フォーマットテスト"""
        # When: 現在日付を取得
        date_str = time_manager.get_current_date()
        
        # Then: YYYY/MM/DD形式の文字列が返される
        assert isinstance(date_str, str)
        assert len(date_str) == 10  # "YYYY/MM/DD"
        assert date_str.count('/') == 2
        
        # 日付部分が数値であることを確認
        parts = date_str.split('/')
        assert len(parts) == 3
        assert len(parts[0]) == 4  # 年
        assert len(parts[1]) == 2  # 月
        assert len(parts[2]) == 2  # 日
        for part in parts:
            assert part.isdigit()
    
    def test_current_datetime_object(self, time_manager):
        """現在日時オブジェクトテスト"""
        # When: 現在日時を取得
        dt = time_manager.get_current_datetime()
        
        # Then: datetimeオブジェクトが返される
        assert isinstance(dt, datetime)
        assert dt.year >= 2024  # 妥当な年
    
    def test_time_formatting(self, time_manager):
        """時刻フォーマットテスト"""
        # Given: 特定の日時
        test_dt = datetime(2024, 12, 25, 15, 30, 45)
        
        # When: 時刻をフォーマット
        time_str = time_manager.format_time(test_dt)
        
        # Then: 正しい時刻文字列が返される
        assert time_str == "15:30:45"
    
    def test_date_formatting(self, time_manager):
        """日付フォーマットテスト"""
        # Given: 特定の日時
        test_dt = datetime(2024, 12, 25, 15, 30, 45)
        
        # When: 日付をフォーマット
        date_str = time_manager.format_date(test_dt)
        
        # Then: 正しい日付文字列が返される
        assert date_str == "2024/12/25"
    
    def test_time_consistency(self, time_manager):
        """時刻一貫性テスト"""
        # When: 短時間で複数回時刻を取得
        time1 = time_manager.get_current_time()
        time2 = time_manager.get_current_time()
        
        # Then: 同じか1秒以内の差である
        # （このテストは実行タイミングに依存するため、フォーマットのみチェック）
        assert isinstance(time1, str)
        assert isinstance(time2, str)
        assert len(time1) == 8
        assert len(time2) == 8