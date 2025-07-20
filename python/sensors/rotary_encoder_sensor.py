# CarBuddy Python版 - ロータリーエンコーダーセンサー管理
import logging
import threading
from typing import Optional, Callable

try:
    from gpiozero import RotaryEncoder
    GPIO_AVAILABLE = True
except ImportError:
    GPIO_AVAILABLE = False
    logging.warning("gpiozero not available - running in mock mode")

logger = logging.getLogger(__name__)

class RotaryEncoderSensor:
    """ロータリーエンコーダーセンサー管理クラス"""
    
    def __init__(self, config: dict):
        self.config = config
        self.encoder: Optional[RotaryEncoder] = None
        self.mock_mode = config.get("MOCK_SENSORS", False) or not GPIO_AVAILABLE
        
        # ピン設定
        self.clk_pin = config.get("ROTARY_ENCODER_CLK_PIN", 17)
        self.dt_pin = config.get("ROTARY_ENCODER_DT_PIN", 18)
        
        # 状態管理
        self.position = 0
        self.last_position = 0
        self._lock = threading.Lock()
        
        # コールバック関数
        self.on_rotate_callback: Optional[Callable[[int], None]] = None
        self.on_clockwise_callback: Optional[Callable[[], None]] = None
        self.on_counter_clockwise_callback: Optional[Callable[[], None]] = None
        
    def initialize(self) -> bool:
        """ロータリーエンコーダーを初期化"""
        if self.mock_mode:
            logger.info("Rotary encoder initialized in mock mode")
            return True
            
        try:
            self.encoder = RotaryEncoder(self.clk_pin, self.dt_pin)
            self.encoder.when_rotated_clockwise = self._on_clockwise
            self.encoder.when_rotated_counter_clockwise = self._on_counter_clockwise
            
            logger.info(f"Rotary encoder initialized successfully (CLK: GPIO{self.clk_pin}, DT: GPIO{self.dt_pin})")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize rotary encoder: {e}")
            self.mock_mode = True
            return False
    
    def _on_clockwise(self):
        """時計回り回転時の内部コールバック"""
        with self._lock:
            self.position += 1
            
        logger.debug(f"Rotary encoder: clockwise rotation, position = {self.position}")
        
        # 外部コールバックを呼び出し
        if self.on_clockwise_callback:
            try:
                self.on_clockwise_callback()
            except Exception as e:
                logger.error(f"Error in clockwise callback: {e}")
                
        if self.on_rotate_callback:
            try:
                self.on_rotate_callback(self.position)
            except Exception as e:
                logger.error(f"Error in rotate callback: {e}")
    
    def _on_counter_clockwise(self):
        """反時計回り回転時の内部コールバック"""
        with self._lock:
            self.position -= 1
            
        logger.debug(f"Rotary encoder: counter-clockwise rotation, position = {self.position}")
        
        # 外部コールバックを呼び出し
        if self.on_counter_clockwise_callback:
            try:
                self.on_counter_clockwise_callback()
            except Exception as e:
                logger.error(f"Error in counter-clockwise callback: {e}")
                
        if self.on_rotate_callback:
            try:
                self.on_rotate_callback(self.position)
            except Exception as e:
                logger.error(f"Error in rotate callback: {e}")
    
    def get_position(self) -> int:
        """現在のポジションを取得"""
        with self._lock:
            return self.position
    
    def reset_position(self):
        """ポジションをリセット"""
        with self._lock:
            self.position = 0
        logger.info("Rotary encoder position reset to 0")
    
    def set_position(self, position: int):
        """ポジションを設定"""
        with self._lock:
            self.position = position
        logger.info(f"Rotary encoder position set to {position}")
    
    def get_relative_change(self) -> int:
        """前回取得時からの変化量を取得"""
        with self._lock:
            change = self.position - self.last_position
            self.last_position = self.position
            return change
    
    def set_on_rotate_callback(self, callback: Callable[[int], None]):
        """回転時のコールバック関数を設定"""
        self.on_rotate_callback = callback
    
    def set_on_clockwise_callback(self, callback: Callable[[], None]):
        """時計回り回転時のコールバック関数を設定"""
        self.on_clockwise_callback = callback
    
    def set_on_counter_clockwise_callback(self, callback: Callable[[], None]):
        """反時計回り回転時のコールバック関数を設定"""
        self.on_counter_clockwise_callback = callback
    
    def cleanup(self):
        """リソースのクリーンアップ"""
        if self.encoder and not self.mock_mode:
            try:
                self.encoder.close()
                logger.info("Rotary encoder resources cleaned up")
            except Exception as e:
                logger.error(f"Error cleaning up rotary encoder: {e}")
        
        self.encoder = None
    
    def get_status(self) -> dict:
        """センサーの状態情報を取得"""
        return {
            "mock_mode": self.mock_mode,
            "position": self.get_position(),
            "clk_pin": self.clk_pin,
            "dt_pin": self.dt_pin,
            "initialized": self.encoder is not None or self.mock_mode
        }
    
    # モック機能（デバッグ・テスト用）
    def mock_rotate_clockwise(self):
        """モック: 時計回り回転をシミュレート"""
        if self.mock_mode:
            self._on_clockwise()
    
    def mock_rotate_counter_clockwise(self):
        """モック: 反時計回り回転をシミュレート"""
        if self.mock_mode:
            self._on_counter_clockwise()