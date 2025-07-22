# CarBuddy Python版 - ディスプレイ管理
import tkinter as tk
from tkinter import ttk, font
from PIL import Image, ImageTk
import os
import logging
import math
from datetime import datetime
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
            "temperature_unit": None,
            "speed_label": None,
            "speed_value": None,
            "speed_unit": None,
            "time_display": None,
            "date_display": None,
            "character_image": None,
            "analog_clock": None
        }
        
        # 表示モード管理
        self.display_mode = "character"  # "character" または "analog_clock"
        self.analog_clock_elements = {
            "face": None,
            "hour_hand": None,
            "minute_hand": None,
            "second_hand": None,
            "center_dot": None,
            "hour_markers": []
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
                highlightthickness=0  # 枠線なし
            )
            self.canvas.pack(fill=tk.BOTH, expand=True)
            
            # 背景色設定
            self.canvas.configure(bg=self.current_background_color)
            
            # キャラクター画像の読み込み
            self._load_character_images()
            
            # UI要素配置
            self._setup_ui_elements()
            
            logger.info("Display initialized successfully")
            return True
            
        except Exception as e:
            logger.error(f"Failed to initialize display: {e}")
            return False
    
    def _exit_fullscreen(self, event=None):
        """フルスクリーン終了"""
        self.root.attributes('-fullscreen', False)
    
    def _load_character_images(self):
        """キャラクター画像をすべて読み込み"""
        image_dir = self.config.get("CHARACTER_IMAGE_DIR")
        image_files = self.config.get("CHARACTER_IMAGE_FILES", {})
        
        if not os.path.exists(image_dir):
            logger.warning(f"Character image directory not found: {image_dir}")
            return
        
        for state, filename in image_files.items():
            file_path = os.path.join(image_dir, filename)
            try:
                if os.path.exists(file_path):
                    # 画像を読み込み、適切なサイズに調整
                    pil_image = Image.open(file_path)
                    # キャラクター画像サイズを300x300に固定
                    pil_image = pil_image.resize((300, 300), Image.Resampling.LANCZOS)
                    self.character_images[state] = ImageTk.PhotoImage(pil_image)
                    logger.debug(f"Loaded character image: {state} from {filename}")
                else:
                    logger.warning(f"Character image file not found: {file_path}")
            except Exception as e:
                logger.error(f"Failed to load character image {filename}: {e}")
    
    def _setup_ui_elements(self):
        """UI要素を配置"""
        colors = self.config["COLORS"]
        
        # タイトル（影付き効果で視認性向上）
        # タイトル影
        self.canvas.create_text(
            402, 22, text="CarBuddy", fill=colors.get("text_shadow", "#000000"),
            font=("Arial", 48, "bold"), anchor="n"
        )
        # タイトル（メイン）
        self.display_elements["title"] = self.canvas.create_text(
            400, 20, text="CarBuddy", fill=colors["text_normal"],
            font=("Arial", 48, "bold"), anchor="n"
        )
        
        # 温度ラベル（影付きで視認性向上）
        fonts = self.config.get("FONTS", {})
        temp_font = fonts.get("temperature", ("Arial", 72, "bold"))
        temp_unit_font = ("Arial", 36, "bold")  # 単位用フォント（半分のサイズ）
        status_font = fonts.get("status", ("Arial", 16, "normal"))
        
        # 温度ラベル影
        self.canvas.create_text(
            402, 102, text="Temperature", fill=colors.get("text_shadow", "#000000"),
            font=status_font, anchor="nw"
        )
        # 温度ラベル（メイン）
        self.display_elements["temperature_label"] = self.canvas.create_text(
            400, 100, text="Temperature", fill=colors["text_normal"],
            font=status_font, anchor="nw"
        )
        
        # 温度値影（数値のみ）
        self.display_elements["temperature_value_shadow"] = self.canvas.create_text(
            402, 82, text="--", fill=colors.get("text_shadow", "#000000"),
            font=temp_font, anchor="nw"
        )
        # 温度値（メイン、数値のみ）
        self.display_elements["temperature_value"] = self.canvas.create_text(
            400, 80, text="--", fill=colors["text_normal"],
            font=temp_font, anchor="nw"
        )
        
        # 温度単位影
        self.display_elements["temperature_unit_shadow"] = self.canvas.create_text(
            552, 102, text="°C", fill=colors.get("text_shadow", "#000000"),
            font=temp_unit_font, anchor="nw"
        )
        # 温度単位（メイン）
        self.display_elements["temperature_unit"] = self.canvas.create_text(
            550, 100, text="°C", fill=colors["text_normal"],
            font=temp_unit_font, anchor="nw"
        )
        
        # 速度表示（影付き）
        speed_font = fonts.get("speed", ("Arial", 72, "bold"))
        speed_unit_font = ("Arial", 36, "bold")  # 単位用フォント（半分のサイズ）
        
        # 速度ラベル影
        self.canvas.create_text(
            402, 242, text="Speed", fill=colors.get("text_shadow", "#000000"),
            font=status_font, anchor="nw"
        )
        # 速度ラベル（メイン）
        self.display_elements["speed_label"] = self.canvas.create_text(
            400, 240, text="Speed", fill=colors["text_normal"],
            font=status_font, anchor="nw"
        )
        
        # 速度値影（数値のみ）
        self.display_elements["speed_value_shadow"] = self.canvas.create_text(
            402, 222, text="0.0", fill=colors.get("text_shadow", "#000000"),
            font=speed_font, anchor="nw"
        )
        # 速度値（メイン、数値のみ）
        self.display_elements["speed_value"] = self.canvas.create_text(
            400, 220, text="0.0", fill=colors["text_normal"],
            font=speed_font, anchor="nw"
        )
        
        # 速度単位影
        self.display_elements["speed_unit_shadow"] = self.canvas.create_text(
            532, 242, text="km/h", fill=colors.get("text_shadow", "#000000"),
            font=speed_unit_font, anchor="nw"
        )
        # 速度単位（メイン）
        self.display_elements["speed_unit"] = self.canvas.create_text(
            530, 240, text="km/h", fill=colors["text_normal"],
            font=speed_unit_font, anchor="nw"
        )
        
        # 時刻表示（影付き、大型化）
        time_font = fonts.get("time", ("Arial", 36, "bold"))
        date_font = fonts.get("date", ("Arial", 36, "bold"))
        
        # 時刻影
        self.display_elements["time_display_shadow"] = self.canvas.create_text(
            402, 372, text="--:--:--", fill=colors.get("text_shadow", "#000000"),
            font=time_font, anchor="nw"
        )
        self.display_elements["time_display"] = self.canvas.create_text(
            400, 370, text="--:--:--", fill=colors["text_time"],
            font=time_font, anchor="nw"
        )
        
        # 日付影（時刻の右隣に配置）
        self.display_elements["date_display_shadow"] = self.canvas.create_text(
            272, 422, text="----/--/--", fill=colors.get("text_shadow", "#000000"),
            font=date_font, anchor="nw"
        )
        self.display_elements["date_display"] = self.canvas.create_text(
            270, 420, text="----/--/--", fill=colors["text_date"],
            font=date_font, anchor="nw"
        )
        
        # デフォルト表示（キャラクター画像）
        self._update_display_mode()
        
        # 初期時刻・日付を設定
        self._set_initial_time_date()
    
    def _set_initial_time_date(self):
        """初期化時に現在の時刻・日付を設定"""
        try:
            from datetime import datetime
            now = datetime.now()
            
            # 現在時刻を設定
            time_str = now.strftime("%H:%M:%S")
            self.update_time_display(time_str)
            
            # 現在日付を設定
            date_str = now.strftime("%Y/%m/%d")
            self.update_date_display(date_str)
            
            logger.info(f"Initial time/date set: {time_str}, {date_str}")
            
        except Exception as e:
            logger.error(f"Failed to set initial time/date: {e}")
    
    def toggle_display_mode(self):
        """表示モードを切り替え（キャラクター ⇔ アナログ時計）"""
        if self.display_mode == "character":
            self.display_mode = "analog_clock"
        else:
            self.display_mode = "character"
        
        self._update_display_mode()
        logger.info(f"Display mode changed to: {self.display_mode}")
    
    def _update_display_mode(self):
        """現在の表示モードに応じて表示を更新"""
        if self.display_mode == "character":
            self._show_character_image()
            self._hide_analog_clock()
        else:
            self._hide_character_image()
            self._show_analog_clock()
    
    def _show_character_image(self):
        """キャラクター画像を表示"""
        if not self.display_elements["character_image"]:
            self._update_character_image("normal")
        else:
            # 既存の画像を表示
            if self.display_elements["character_image"]:
                self.canvas.itemconfig(self.display_elements["character_image"], state='normal')
    
    def _hide_character_image(self):
        """キャラクター画像を非表示"""
        if self.display_elements["character_image"]:
            self.canvas.itemconfig(self.display_elements["character_image"], state='hidden')
    
    def _show_analog_clock(self):
        """アナログ時計を表示"""
        self._draw_analog_clock()
        self._update_analog_clock()
    
    def _hide_analog_clock(self):
        """アナログ時計を非表示"""
        for element_id in self.analog_clock_elements.values():
            if isinstance(element_id, list):
                for item_id in element_id:
                    if item_id:
                        self.canvas.itemconfig(item_id, state='hidden')
            elif element_id:
                self.canvas.itemconfig(element_id, state='hidden')
    
    def _draw_analog_clock(self):
        """アナログ時計を描画"""
        # 時計の中心位置とサイズ
        center_x, center_y = 150, 250
        radius = 80
        
        colors = self.config["COLORS"]
        
        # 時計の文字盤（円）
        if not self.analog_clock_elements["face"]:
            self.analog_clock_elements["face"] = self.canvas.create_oval(
                center_x - radius, center_y - radius,
                center_x + radius, center_y + radius,
                outline=colors["text_normal"], width=3, fill="", state='normal'
            )
        else:
            self.canvas.itemconfig(self.analog_clock_elements["face"], state='normal')
        
        # 時間マーカー（12時、3時、6時、9時）
        if not self.analog_clock_elements["hour_markers"]:
            for hour in range(0, 12, 3):
                angle = math.radians(hour * 30 - 90)  # 12時を0度とする
                outer_x = center_x + (radius - 10) * math.cos(angle)
                outer_y = center_y + (radius - 10) * math.sin(angle)
                inner_x = center_x + (radius - 20) * math.cos(angle)
                inner_y = center_y + (radius - 20) * math.sin(angle)
                
                marker_id = self.canvas.create_line(
                    inner_x, inner_y, outer_x, outer_y,
                    fill=colors["text_normal"], width=3, state='normal'
                )
                self.analog_clock_elements["hour_markers"].append(marker_id)
        else:
            for marker_id in self.analog_clock_elements["hour_markers"]:
                self.canvas.itemconfig(marker_id, state='normal')
        
        # 中心の点
        if not self.analog_clock_elements["center_dot"]:
            self.analog_clock_elements["center_dot"] = self.canvas.create_oval(
                center_x - 4, center_y - 4, center_x + 4, center_y + 4,
                fill=colors["text_normal"], outline=colors["text_normal"], state='normal'
            )
        else:
            self.canvas.itemconfig(self.analog_clock_elements["center_dot"], state='normal')
    
    def _update_analog_clock(self):
        """アナログ時計の針を現在時刻に更新"""
        if self.display_mode != "analog_clock":
            return
            
        now = datetime.now()
        center_x, center_y = 150, 250
        colors = self.config["COLORS"]
        
        # 針の角度計算
        hour_angle = math.radians((now.hour % 12) * 30 + now.minute * 0.5 - 90)
        minute_angle = math.radians(now.minute * 6 - 90)
        second_angle = math.radians(now.second * 6 - 90)
        
        # 既存の針を削除
        if self.analog_clock_elements["hour_hand"]:
            self.canvas.delete(self.analog_clock_elements["hour_hand"])
        if self.analog_clock_elements["minute_hand"]:
            self.canvas.delete(self.analog_clock_elements["minute_hand"])
        if self.analog_clock_elements["second_hand"]:
            self.canvas.delete(self.analog_clock_elements["second_hand"])
        
        # 時針
        hour_x = center_x + 40 * math.cos(hour_angle)
        hour_y = center_y + 40 * math.sin(hour_angle)
        self.analog_clock_elements["hour_hand"] = self.canvas.create_line(
            center_x, center_y, hour_x, hour_y,
            fill=colors["text_normal"], width=4, state='normal'
        )
        
        # 分針
        minute_x = center_x + 60 * math.cos(minute_angle)
        minute_y = center_y + 60 * math.sin(minute_angle)
        self.analog_clock_elements["minute_hand"] = self.canvas.create_line(
            center_x, center_y, minute_x, minute_y,
            fill=colors["text_normal"], width=3, state='normal'
        )
        
        # 秒針
        second_x = center_x + 70 * math.cos(second_angle)
        second_y = center_y + 70 * math.sin(second_angle)
        self.analog_clock_elements["second_hand"] = self.canvas.create_line(
            center_x, center_y, second_x, second_y,
            fill=colors["text_hot"], width=1, state='normal'
        )
    
    def update_temperature_display(self, temperature: float):
        """温度表示を更新"""
        if not self.canvas or not self.display_elements["temperature_value"] or not self.root:
            return
        
        # メインスレッドから呼び出されていない場合は安全にスケジュール
        try:
            if self.root and hasattr(self.root, 'winfo_exists') and self.root.winfo_exists():
                self._safe_update_temperature(temperature)
            else:
                return
        except Exception as e:
            logger.warning(f"Temperature display update skipped: {e}")
            return
    
    def _safe_update_temperature(self, temperature: float):
        """安全な温度表示更新（実際の更新処理）"""        
        colors = self.config["COLORS"]
        temp_threshold = self.config["TEMP_THRESHOLDS"]["hot_warning"]
        
        # 温度に応じた文字色
        text_color = colors["text_hot"] if temperature >= temp_threshold else colors["text_normal"]
        
        # 温度値更新（数値のみ、影とメイン両方）
        temp_text = f"{temperature:.1f}"
        self.canvas.itemconfig(
            self.display_elements["temperature_value_shadow"],
            text=temp_text
        )
        self.canvas.itemconfig(
            self.display_elements["temperature_value"], 
            text=temp_text,
            fill=text_color
        )
        
        # 温度単位更新（動的位置計算付き、影とメイン両方）
        if self.display_elements["temperature_unit"]:
            # 数値の幅を計算して単位位置を決定
            fonts = self.config.get("FONTS", {})
            temp_font = fonts.get("temperature", ("Arial", 72, "bold"))
            text_width = self._calculate_text_width(temp_text, temp_font)
            
            # 数値の開始位置 + 幅 + 少しの間隔
            unit_x = 400 + text_width + 5
            unit_x_shadow = unit_x + 2
            
            # 単位位置を更新
            self.canvas.coords(self.display_elements["temperature_unit_shadow"], unit_x_shadow, 102)
            self.canvas.coords(self.display_elements["temperature_unit"], unit_x, 100)
            
            # 単位の色を更新
            self.canvas.itemconfig(
                self.display_elements["temperature_unit_shadow"],
                fill=colors.get("text_shadow", "#000000")
            )
            self.canvas.itemconfig(
                self.display_elements["temperature_unit"],
                fill=text_color
            )
        
        # 背景色更新（ホット表示かどうかによって背景色を変更）
        new_bg_color = colors["background_hot"] if temperature >= temp_threshold else colors["background_cold"]
        if new_bg_color != self.current_background_color:
            self.canvas.configure(bg=new_bg_color)
            self.current_background_color = new_bg_color
        
        # キャラクター画像更新（キャラクターモードの場合）
        if self.display_mode == "character":
            character_state = "hot" if temperature >= temp_threshold else "normal"
            self._update_character_image(character_state)
    
    def update_speed_display(self, speed: float):
        """速度表示を更新"""
        if not self.canvas or not self.display_elements["speed_value"]:
            return
        
        colors = self.config["COLORS"]
        
        # 速度値更新（数値のみ、影とメイン両方）
        speed_text = f"{speed:.1f}"
        self.canvas.itemconfig(
            self.display_elements["speed_value_shadow"],
            text=speed_text
        )
        self.canvas.itemconfig(
            self.display_elements["speed_value"], 
            text=speed_text
        )
        
        # 速度単位更新（動的位置計算付き、影とメイン両方）
        if self.display_elements["speed_unit"]:
            # 数値の幅を計算して単位位置を決定
            fonts = self.config.get("FONTS", {})
            speed_font = fonts.get("speed", ("Arial", 72, "bold"))
            text_width = self._calculate_text_width(speed_text, speed_font)
            
            # 数値の開始位置 + 幅 + 少しの間隔
            unit_x = 400 + text_width + 5
            unit_x_shadow = unit_x + 2
            
            # 単位位置を更新
            self.canvas.coords(self.display_elements["speed_unit_shadow"], unit_x_shadow, 242)
            self.canvas.coords(self.display_elements["speed_unit"], unit_x, 240)
    
    def _calculate_text_width(self, text: str, font_spec: tuple) -> int:
        """テキストの幅を計算"""
        try:
            # フォント仕様からtkinter.fontオブジェクトを作成
            font_name, font_size, font_style = font_spec
            tk_font = font.Font(family=font_name, size=font_size, weight=font_style)
            return tk_font.measure(text)
        except Exception as e:
            logger.warning(f"Failed to calculate text width: {e}")
            # フォールバック: 推定値を返す
            return len(text) * (font_spec[1] // 2)  # 大まかな推定
    
    def update_time_display(self, time_str: str):
        """時刻表示を更新"""
        if not self.canvas or not self.display_elements["time_display"]:
            return
            
        self.canvas.itemconfig(
            self.display_elements["time_display_shadow"],
            text=time_str
        )
        self.canvas.itemconfig(
            self.display_elements["time_display"], 
            text=time_str
        )
        
        # アナログ時計モードの場合、針も更新
        if self.display_mode == "analog_clock":
            self._update_analog_clock()
    
    def update_date_display(self, date_str: str):
        """日付表示を更新"""
        if not self.canvas or not self.display_elements["date_display"]:
            return
            
        self.canvas.itemconfig(
            self.display_elements["date_display_shadow"],
            text=date_str
        )
        self.canvas.itemconfig(
            self.display_elements["date_display"], 
            text=date_str
        )
    
    def _update_character_image(self, state: str):
        """キャラクター画像を更新"""
        if state not in self.character_images:
            logger.warning(f"Character image not found for state: {state}")
            return
        
        # 既存の画像を削除
        if self.display_elements["character_image"]:
            self.canvas.delete(self.display_elements["character_image"])
        
        # 新しい画像を配置（左側中央）
        self.display_elements["character_image"] = self.canvas.create_image(
            150, 250,  # x=150（左側）, y=250（中央）
            image=self.character_images[state],
            anchor="center"
        )
        
        logger.debug(f"Character image updated to state: {state}")
    
    def show_splash_screen(self, message: str = "CarBuddy"):
        """スプラッシュ画面を表示"""
        if not self.canvas:
            return
            
        # 背景を黒に
        self.canvas.configure(bg="#000000")
        
        # 既存の要素を一時的に非表示に
        for element_id in self.display_elements.values():
            if element_id:
                self.canvas.itemconfig(element_id, state='hidden')
        
        # スプラッシュテキスト表示
        fonts = self.config.get("FONTS", {})
        splash_font = fonts.get("splash", ("Arial", 32, "bold"))
        
        self.display_elements["splash_text"] = self.canvas.create_text(
            400, 240, text=message, fill="#FFFFFF",
            font=splash_font, anchor="center"
        )
        
        # 画面更新を強制
        self.canvas.update()
        
        logger.info(f"Splash screen shown: {message}")
    
    def update_splash_message(self, message: str):
        """スプラッシュ画面のメッセージを更新"""
        if not self.canvas or "splash_text" not in self.display_elements:
            return
        
        try:
            self.canvas.itemconfig(
                self.display_elements["splash_text"],
                text=message
            )
            self.canvas.update()
        except Exception as e:
            logger.warning(f"Failed to update splash message: {e}")
    
    def hide_splash_screen_manual(self):
        """スプラッシュ画面を手動で非表示にする"""
        if not self.canvas:
            return
            
        # スプラッシュテキストを削除
        if "splash_text" in self.display_elements and self.display_elements["splash_text"]:
            self.canvas.delete(self.display_elements["splash_text"])
            del self.display_elements["splash_text"]
        
        # 背景色を通常に戻す
        self.canvas.configure(bg=self.current_background_color)
        
        # 通常の要素を表示
        for element_id in self.display_elements.values():
            if element_id:
                self.canvas.itemconfig(element_id, state='normal')
        
        # 現在の表示モードを適用
        self._update_display_mode()
        
        logger.info("Splash screen hidden manually")
    
    def hide_splash_screen(self, delay_ms: int = 3000):
        """スプラッシュ画面を指定時間後に非表示（非推奨）"""
        # この機能は手動制御に置き換えられました
        self.hide_splash_screen_manual()
    
    def run_main_loop(self):
        """メインGUIループを実行"""
        if self.root:
            logger.info("Starting Tkinter main loop")
            self.root.mainloop()
    
    def destroy(self):
        """リソースを解放してウィンドウを閉じる"""
        try:
            if self.root:
                self.root.quit()
                self.root.destroy()
                self.root = None
            logger.info("Display manager destroyed")
        except Exception as e:
            logger.error(f"Error destroying display manager: {e}")