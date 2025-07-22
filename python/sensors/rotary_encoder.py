# CarBuddy Python版 - ロータリーエンコーダー管理
import logging
import time
from typing import Optional, Callable

try:
    from gpiozero import RotaryEncoder
    GPIO_AVAILABLE = True
except ImportError:
    GPIO_AVAILABLE = False
    logging.warning("gpiozero not available - running in mock mode")

logger = logging.getLogger(__name__)

class RotaryEncoderInput:
    """ロータリーエンコーダー入力管理クラス"""
    
    def __init__(self, config: dict):
        self.config = config
        self.encoder: Optional[RotaryEncoder] = None
        self.mock_mode = config.get("MOCK_SENSORS", False) or not GPIO_AVAILABLE
        self.position = 0
        self.last_position = 0
        
        # コールバック関数
        self.on_rotate_callback: Optional[Callable[[int], None]] = None
        
        # GPIO設定
        self.clk_pin = config.get("ROTARY_ENCODER_CLK_PIN", 17)  # GPIO17
        self.dt_pin = config.get("ROTARY_ENCODER_DT_PIN", 18)   # GPIO18
        
    def initialize(self) -> bool:
        """ロータリーエンコーダーを初期化"""
        if self.mock_mode:
            logger.info("Rotary encoder initialized in mock mode")
            return True
            
        try:
            self.encoder = RotaryEncoder(self.clk_pin, self.dt_pin)
            self.encoder.when_rotated_clockwise = self._on_rotate_clockwise
            self.encoder.when_rotated_counter_clockwise = self._on_rotate_counter_clockwise
            
            logger.info(f"Rotary encoder initialized successfully (CLK: GPIO{self.clk_pin}, DT: GPIO{self.dt_pin})")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize rotary encoder: {e}")
            logger.info("Falling back to mock mode")
            self.mock_mode = True
            return True  # モックモードで継続
    
    def set_rotation_callback(self, callback: Callable[[int], None]):
        """回転時のコールバック関数を設定"""
        self.on_rotate_callback = callback
        
    def _on_rotate_clockwise(self):
        """時計回り回転時の処理"""
        self.position += 1
        logger.debug(f"Rotary encoder: clockwise, position = {self.position}")
        
        if self.on_rotate_callback:
            try:
                self.on_rotate_callback(1)  # 正の値で時計回り
            except Exception as e:
                logger.error(f"Error in rotation callback: {e}")
    
    def _on_rotate_counter_clockwise(self):
        """反時計回り回転時の処理"""
        self.position -= 1
        logger.debug(f"Rotary encoder: counter-clockwise, position = {self.position}")
        
        if self.on_rotate_callback:
            try:
                self.on_rotate_callback(-1)  # 負の値で反時計回り
            except Exception as e:
                logger.error(f"Error in rotation callback: {e}")
    
    def get_position(self) -> int:
        """現在の位置を取得"""
        return self.position
    
    def reset_position(self):
        """位置をリセット"""
        self.position = 0
        self.last_position = 0
        logger.debug("Rotary encoder position reset")
    
    def is_available(self) -> bool:
        """エンコーダーが利用可能かチェック"""
        if self.mock_mode:
            return True
        return self.encoder is not None
    
    def cleanup(self):
        """リソース解放"""
        if self.encoder:
            try:
                self.encoder.close()
                logger.info("Rotary encoder resources cleaned up")
            except Exception as e:
                logger.warning(f"Error cleaning up rotary encoder: {e}")
            finally:
                self.encoder = None
    
    def get_sensor_info(self) -> dict:
        """センサー情報を取得"""
        if self.mock_mode:
            return {
                "type": "Mock Rotary Encoder",
                "pins": f"CLK: GPIO{self.clk_pin}, DT: GPIO{self.dt_pin}",
                "status": "mock_mode"
            }
        
        if self.encoder:
            return {
                "type": "Rotary Encoder",
                "pins": f"CLK: GPIO{self.clk_pin}, DT: GPIO{self.dt_pin}",
                "status": "active",
                "position": self.position
            }
        
        return {
            "type": "Rotary Encoder",
            "pins": "unknown",
            "status": "inactive"
        }