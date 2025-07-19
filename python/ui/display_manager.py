# CarBuddy Python版 - ディスプレイ管理
import tkinter as tk
from tkinter import ttk
from PIL import Image, ImageTk
import os
import logging
from typing import Optional, Dict, Any

logger = logging.getLogger(__name__)

class DisplayManager:
    """画面表示とPNG画像管理を担当するクラス"""
    
    def __init__(self, config: Dict[str, Any]):
        self.config = config
        self.root: Optional[tk.Tk] = None
        self.canvas: Optional[tk.Canvas] = None
        self.character_images: Dict[str, ImageTk.PhotoImage] = {}
        self.current_background_color = config["COLORS"]["background_cold"]
        
        # 表示要素のID管理
        self.display_elements = {
            "title": None,
            "temperature_label": None,
            "temperature_value": None,
            "speed_label": None,
            "speed_value": None,
            "time_display": None,
            "date_display": None,
            "character_image": None
        }
        
    def initialize_display(self) -> bool:
        """ディスプレイシステムを初期化"""
        try:
            # メインウィンドウ作成
            self.root = tk.Tk()
            self.root.title("CarBuddy")
            
            # フルスクリーン設定
            if self.config.get("FULLSCREEN", True):
                self.root.attributes('-fullscreen', True)
                self.root.bind('<Escape>', self._exit_fullscreen)
            
            # ウィンドウサイズ設定
            screen_width = self.config.get("SCREEN_WIDTH", 800)
            screen_height = self.config.get("SCREEN_HEIGHT", 480)
            self.root.geometry(f"{screen_width}x{screen_height}")
            
            # キャンバス作成
            self.canvas = tk.Canvas(
                self.root,
                width=screen_width,
                height=screen_height,
                bg=self.current_background_color,
                highlightthickness=0
            )
            self.canvas.pack(fill=tk.BOTH, expand=True)
            
            # PNG画像を読み込み
            self._load_character_images()
            
            # 初期UI描画
            self._draw_initial_ui()
            
            logger.info("Display system initialized successfully")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize display: {e}")
            return False
    
    def _load_character_images(self):
        """キャラクター画像（PNG）を読み込み"""
        image_dir = self.config.get("CHARACTER_IMAGE_DIR", "images")
        image_files = self.config.get("CHARACTER_IMAGE_FILES", {})
        
        for image_key, filename in image_files.items():
            image_path = os.path.join(image_dir, filename)
            
            try:
                if os.path.exists(image_path):
                    # PNG画像を読み込み、リサイズ
                    pil_image = Image.open(image_path)
                    pil_image = pil_image.resize((180, 180), Image.Resampling.LANCZOS)
                    
                    # tkinter用に変換
                    self.character_images[image_key] = ImageTk.PhotoImage(pil_image)
                    logger.info(f"Loaded character image: {filename}")
                else:
                    logger.warning(f"Character image not found: {image_path}")
                    
            except Exception as e:
                logger.error(f"Failed to load image {filename}: {e}")
    
    def _draw_initial_ui(self):
        """初期UI要素を描画"""
        if not self.canvas:
            return
            
        colors = self.config["COLORS"]
        
        # タイトル
        self.display_elements["title"] = self.canvas.create_text(
            50, 20, text="CarBuddy", fill=colors["text_normal"],
            font=("Arial", 24, "bold"), anchor="nw"
        )
        
        # 温度ラベル
        self.display_elements["temperature_label"] = self.canvas.create_text(
            400, 50, text="Temp:", fill=colors["text_normal"],
            font=("Arial", 20, "bold"), anchor="nw"
        )
        
        # 温度値（初期値）
        self.display_elements["temperature_value"] = self.canvas.create_text(
            400, 80, text="--°C", fill=colors["text_normal"],
            font=("Arial", 18), anchor="nw"
        )
        
        # 速度ラベル
        self.display_elements["speed_label"] = self.canvas.create_text(
            400, 150, text="Speed:", fill=colors["text_normal"],
            font=("Arial", 20, "bold"), anchor="nw"
        )
        
        # 速度値（初期値）
        self.display_elements["speed_value"] = self.canvas.create_text(
            400, 180, text="0.0 km/h", fill=colors["text_normal"],
            font=("Arial", 18), anchor="nw"
        )
        
        # 時刻表示（初期値）
        self.display_elements["time_display"] = self.canvas.create_text(
            50, 420, text="--:--:--", fill=colors["text_time"],
            font=("Arial", 16), anchor="nw"
        )
        
        # 日付表示（初期値）
        self.display_elements["date_display"] = self.canvas.create_text(
            200, 420, text="----/--/--", fill=colors["text_date"],
            font=("Arial", 16), anchor="nw"
        )
        
        # デフォルトキャラクター画像
        self._update_character_image("normal")
    
    def update_temperature_display(self, temperature: float):
        """温度表示を更新"""
        if not self.canvas or not self.display_elements["temperature_value"]:
            return
            
        colors = self.config["COLORS"]
        temp_threshold = self.config["TEMP_THRESHOLDS"]["hot_warning"]
        
        # 温度に応じた文字色
        text_color = colors["text_hot"] if temperature >= temp_threshold else colors["text_normal"]
        
        # 温度値更新
        self.canvas.itemconfig(
            self.display_elements["temperature_value"],
            text=f"{temperature:.1f}°C",
            fill=text_color
        )
        
        # 背景色更新
        self._update_background_color(temperature)
        
        # キャラクター画像更新
        if temperature >= temp_threshold:
            self._update_character_image("hot")
        else:
            self._update_character_image("normal")
    
    def update_speed_display(self, speed: float):
        """速度表示を更新"""
        if not self.canvas or not self.display_elements["speed_value"]:
            return
            
        self.canvas.itemconfig(
            self.display_elements["speed_value"],
            text=f"{abs(speed):.1f} km/h"
        )
    
    def update_time_display(self, time_str: str):
        """時刻表示を更新"""
        if not self.canvas or not self.display_elements["time_display"]:
            return
            
        self.canvas.itemconfig(
            self.display_elements["time_display"],
            text=time_str
        )
    
    def update_date_display(self, date_str: str):
        """日付表示を更新"""
        if not self.canvas or not self.display_elements["date_display"]:
            return
            
        self.canvas.itemconfig(
            self.display_elements["date_display"],
            text=date_str
        )
    
    def _update_character_image(self, image_key: str):
        """キャラクター画像を更新"""
        if image_key not in self.character_images:
            logger.warning(f"Character image not found: {image_key}")
            return
            
        # 既存の画像を削除
        if self.display_elements["character_image"]:
            self.canvas.delete(self.display_elements["character_image"])
        
        # 新しい画像を配置
        self.display_elements["character_image"] = self.canvas.create_image(
            150, 250,  # 中央左寄りに配置
            image=self.character_images[image_key]
        )
    
    def _update_background_color(self, temperature: float):
        """温度に応じて背景色を更新"""
        colors = self.config["COLORS"]
        transition_temp = self.config["TEMP_THRESHOLDS"]["transition"]
        hot_temp = self.config["TEMP_THRESHOLDS"]["hot_warning"]
        
        if temperature >= hot_temp:
            new_color = colors["background_hot"]
        elif temperature >= transition_temp:
            # グラデーション計算（青→赤）
            ratio = (temperature - transition_temp) / (hot_temp - transition_temp)
            new_color = self._interpolate_color(
                colors["background_cold"],
                colors["background_hot"],
                ratio
            )
        else:
            new_color = colors["background_cold"]
        
        if new_color != self.current_background_color:
            self.canvas.configure(bg=new_color)
            self.current_background_color = new_color
    
    def _interpolate_color(self, color1: str, color2: str, ratio: float) -> str:
        """2つの色の間を補間"""
        # 16進数カラーをRGBに変換
        rgb1 = tuple(int(color1[i:i+2], 16) for i in (1, 3, 5))
        rgb2 = tuple(int(color2[i:i+2], 16) for i in (1, 3, 5))
        
        # 補間計算
        rgb_result = tuple(
            int(rgb1[i] + (rgb2[i] - rgb1[i]) * ratio)
            for i in range(3)
        )
        
        # 16進数に戻す
        return f"#{rgb_result[0]:02x}{rgb_result[1]:02x}{rgb_result[2]:02x}"
    
    def _exit_fullscreen(self, event=None):
        """フルスクリーン終了"""
        if self.root:
            self.root.attributes('-fullscreen', False)
    
    def show_splash_screen(self):
        """スプラッシュ画面表示"""
        if not self.canvas:
            return
            
        # 一時的なスプラッシュ表示
        splash_text = self.canvas.create_text(
            self.config["SCREEN_WIDTH"] // 2,
            self.config["SCREEN_HEIGHT"] // 2,
            text="CarBuddy\nStarting...",
            fill=self.config["COLORS"]["text_normal"],
            font=("Arial", 32, "bold"),
            justify="center"
        )
        
        self.root.update()
        
        # 2秒後に削除
        self.root.after(2000, lambda: self.canvas.delete(splash_text))
    
    def run_main_loop(self):
        """メインループ開始"""
        if self.root:
            self.root.mainloop()
    
    def destroy(self):
        """リソース解放"""
        if self.root:
            self.root.destroy()