# CarBuddy Python版 - 時刻管理
import logging
from datetime import datetime
from typing import Optional

logger = logging.getLogger(__name__)

class TimeManager:
    """時刻・日付管理クラス"""
    
    def __init__(self, config: dict):
        self.config = config
        self.time_format = "%H:%M:%S"
        self.date_format = "%Y/%m/%d"
        
    def get_current_time(self) -> str:
        """現在時刻を文字列で取得"""
        try:
            now = datetime.now()
            return now.strftime(self.time_format)
        except Exception as e:
            logger.error(f"Failed to get current time: {e}")
            return "--:--:--"
    
    def get_current_date(self) -> str:
        """現在日付を文字列で取得"""
        try:
            now = datetime.now()
            return now.strftime(self.date_format)
        except Exception as e:
            logger.error(f"Failed to get current date: {e}")
            return "----/--/--"
    
    def get_current_datetime(self) -> Optional[datetime]:
        """現在の日時をdatetimeオブジェクトで取得"""
        try:
            return datetime.now()
        except Exception as e:
            logger.error(f"Failed to get current datetime: {e}")
            return None
    
    def format_time(self, dt: datetime) -> str:
        """datetimeオブジェクトを時刻文字列に変換"""
        try:
            return dt.strftime(self.time_format)
        except Exception as e:
            logger.error(f"Failed to format time: {e}")
            return "--:--:--"
    
    def format_date(self, dt: datetime) -> str:
        """datetimeオブジェクトを日付文字列に変換"""
        try:
            return dt.strftime(self.date_format)
        except Exception as e:
            logger.error(f"Failed to format date: {e}")
            return "----/--/--"