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
                bg="black",  # グラデーション背景用に黒背景
                highlightthickness=0
            )
            self.canvas.pack(fill=tk.BOTH, expand=True)
            
            # グラデーション背景用の矩形ID
            self.background_gradient_items = []
            
            # アニメーション制御（設定から取得）
            self.target_background_color = self.current_background_color
            self.animation_steps = 20  # アニメーション段数
            self.animation_duration = self.config.get("ANIMATIONS", {}).get("background_transition_duration", 300)
            self.animation_timer = None
            
            # 数値表示用（滑らかな変化）
            self.current_temp_display = 0.0
            self.target_temp_display = 0.0
            self.current_speed_display = 0.0
            self.target_speed_display = 0.0
            
            # スプラッシュ画面制御
            self.splash_text_id = None
            self.splash_background_id = None
            self.splash_animation_timer = None
            self.splash_fade_step = 0
            self.splash_fade_steps = 30  # フェードアニメーション段数
            self.splash_fade_duration = 1000  # フェード時間（ms）
            
            
            # PNG画像を読み込み
            self._load_character_images()
            
            # 初期グラデーション背景描画
            self._draw_gradient_background(self.current_background_color)
            self.current_background_color = self.current_background_color  # 確実に設定
            
            
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
        
        # 温度ラベル（影付きで視認性向上）
        fonts = self.config.get("FONTS", {})
        temp_font = fonts.get("temperature", ("Arial", 72, "bold"))
        status_font = fonts.get("status", ("Arial", 16, "normal"))
        
        # 温度ラベル影
        self.canvas.create_text(
            402, 52, text="Temp:", fill=colors.get("text_shadow", "#000000"),
            font=status_font, anchor="nw"
        )
        self.display_elements["temperature_label"] = self.canvas.create_text(
            400, 50, text="Temp:", fill=colors["text_normal"],
            font=status_font, anchor="nw"
        )
        
        # 温度値影
        self.display_elements["temperature_value_shadow"] = self.canvas.create_text(
            402, 82, text="--°C", fill=colors.get("text_shadow", "#000000"),
            font=temp_font, anchor="nw"
        )
        # 温度値（メイン）
        self.display_elements["temperature_value"] = self.canvas.create_text(
            400, 80, text="--°C", fill=colors["text_normal"],
            font=temp_font, anchor="nw"
        )
        
        # 速度表示（影付き）
        speed_font = fonts.get("speed", ("Arial", 48, "normal"))
        
        # 速度ラベル影
        self.canvas.create_text(
            402, 192, text="Speed:", fill=colors.get("text_shadow", "#000000"),
            font=status_font, anchor="nw"
        )
        self.display_elements["speed_label"] = self.canvas.create_text(
            400, 190, text="Speed:", fill=colors["text_normal"],
            font=status_font, anchor="nw"
        )
        
        # 速度値影
        self.display_elements["speed_value_shadow"] = self.canvas.create_text(
            402, 222, text="0.0 km/h", fill=colors.get("text_shadow", "#000000"),
            font=speed_font, anchor="nw"
        )
        # 速度値（メイン）
        self.display_elements["speed_value"] = self.canvas.create_text(
            400, 220, text="0.0 km/h", fill=colors["text_normal"],
            font=speed_font, anchor="nw"
        )
        
        # 時刻表示（影付き、大型化）
        time_font = fonts.get("time", ("Arial", 36, "bold"))
        date_font = fonts.get("date", ("Arial", 24, "normal"))
        
        # 時刻影
        self.display_elements["time_display_shadow"] = self.canvas.create_text(
            52, 422, text="--:--:--", fill=colors.get("text_shadow", "#000000"),
            font=time_font, anchor="nw"
        )
        self.display_elements["time_display"] = self.canvas.create_text(
            50, 420, text="--:--:--", fill=colors["text_time"],
            font=time_font, anchor="nw"
        )
        
        # 日付影（時刻の右隣に配置）
        self.display_elements["date_display_shadow"] = self.canvas.create_text(
            252, 422, text="----/--/--", fill=colors.get("text_shadow", "#000000"),
            font=date_font, anchor="nw"
        )
        self.display_elements["date_display"] = self.canvas.create_text(
            250, 420, text="----/--/--", fill=colors["text_date"],
            font=date_font, anchor="nw"
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
        
        # 温度値更新（影とメイン両方）
        temp_text = f"{temperature:.1f}°C"
        self.canvas.itemconfig(
            self.display_elements["temperature_value_shadow"],
            text=temp_text
        )
        self.canvas.itemconfig(
            self.display_elements["temperature_value"],
            text=temp_text,
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
        
        # 速度値更新（影とメイン両方）
        speed_text = f"{abs(speed):.1f} km/h"
        self.canvas.itemconfig(
            self.display_elements["speed_value_shadow"],
            text=speed_text
        )
        self.canvas.itemconfig(
            self.display_elements["speed_value"],
            text=speed_text
        )
    
    def update_time_display(self, time_str: str):
        """時刻表示を更新"""
        if not self.canvas or not self.display_elements["time_display"]:
            return
        
        # 時刻更新（影とメイン両方）
        self.canvas.itemconfig(
            self.display_elements["time_display_shadow"],
            text=time_str
        )
        self.canvas.itemconfig(
            self.display_elements["time_display"],
            text=time_str
        )
    
    def update_date_display(self, date_str: str):
        """日付表示を更新"""
        if not self.canvas or not self.display_elements["date_display"]:
            return
        
        # 日付更新（影とメイン両方）
        self.canvas.itemconfig(
            self.display_elements["date_display_shadow"],
            text=date_str
        )
        self.canvas.itemconfig(
            self.display_elements["date_display"],
            text=date_str
        )
    
    def _update_character_image(self, image_key: str):
        """キャラクター画像を更新"""
        if image_key not in self.character_images:
            logger.warning(f"Character image not found: {image_key}")
            return
            
        # 既存の画像がある場合は画像のみを変更（ちらつき防止）
        if self.display_elements["character_image"]:
            self.canvas.itemconfig(
                self.display_elements["character_image"],
                image=self.character_images[image_key]
            )
        else:
            # 初回のみ新しい画像を作成
            self.display_elements["character_image"] = self.canvas.create_image(
                150, 250,  # 中央左寄りに配置
                image=self.character_images[image_key]
            )
    
    def _update_background_color(self, temperature: float):
        """温度に応じてグラデーション背景色を更新"""
        colors = self.config["COLORS"]
        transition_temp = self.config["TEMP_THRESHOLDS"]["transition"]
        hot_temp = self.config["TEMP_THRESHOLDS"]["hot_warning"]
        
        if temperature >= hot_temp:
            target_color = colors["background_hot"]
        elif temperature >= transition_temp:
            # グラデーション計算（青→赤）
            ratio = (temperature - transition_temp) / (hot_temp - transition_temp)
            target_color = self._interpolate_color(
                colors["background_cold"],
                colors["background_hot"],
                ratio
            )
        else:
            target_color = colors["background_cold"]
        
        # 目標色が変わった場合のみアニメーション開始
        if target_color != self.target_background_color:
            self.target_background_color = target_color
            self._start_color_animation()
    
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
    
    
    
    def _start_color_animation(self):
        """背景色アニメーションを開始"""
        # 既存のアニメーションがあればキャンセル
        if self.animation_timer:
            self.root.after_cancel(self.animation_timer)
        
        # アニメーション開始
        self.animation_step = 0
        self.animation_start_color = self.current_background_color
        self._animate_color_step()
    
    def _animate_color_step(self):
        """色アニメーションの1ステップ実行"""
        if self.animation_step >= self.animation_steps:
            # アニメーション完了
            self.current_background_color = self.target_background_color
            self._update_gradient_colors(self.current_background_color)
            self.animation_timer = None
            return
        
        # 現在のアニメーション進行度（0.0 〜 1.0）
        progress = self.animation_step / self.animation_steps
        
        # イージング関数適用（滑らかな変化）
        eased_progress = self._ease_in_out(progress)
        
        # 中間色を計算
        intermediate_color = self._interpolate_color(
            self.animation_start_color,
            self.target_background_color,
            eased_progress
        )
        
        # グラデーション背景を更新（既存の矩形の色のみ変更）
        self._update_gradient_colors(intermediate_color)
        
        # 次のステップをスケジュール
        self.animation_step += 1
        step_delay = self.animation_duration // self.animation_steps
        self.animation_timer = self.root.after(step_delay, self._animate_color_step)
    
    def _ease_in_out(self, t: float) -> float:
        """イージング関数（滑らかな加速・減速）"""
        return t * t * (3.0 - 2.0 * t)
    
    def _update_gradient_colors(self, base_color: str):
        """既存のグラデーション矩形の色のみを更新"""
        if not self.background_gradient_items:
            # 初回の場合は新規作成
            self._draw_gradient_background(base_color)
            return
        
        # ベース色から明るい色と暗い色を計算
        rgb_base = tuple(int(base_color[i:i+2], 16) for i in (1, 3, 5))
        
        for i, rect_id in enumerate(self.background_gradient_items):
            # 上部は明るく、下部は暗く
            lightness_factor = 1.3 - (i / len(self.background_gradient_items)) * 0.6
            
            # RGB値を調整
            rgb_adjusted = tuple(
                min(255, max(0, int(rgb_base[j] * lightness_factor)))
                for j in range(3)
            )
            
            color = f"#{rgb_adjusted[0]:02x}{rgb_adjusted[1]:02x}{rgb_adjusted[2]:02x}"
            
            # 既存の矩形の色を変更
            self.canvas.itemconfig(rect_id, fill=color, outline=color)
    
    def _draw_gradient_background(self, base_color: str):
        """グラデーション背景を描画"""
        # 既存のグラデーション要素を削除
        for item in self.background_gradient_items:
            self.canvas.delete(item)
        self.background_gradient_items.clear()
        
        # 画面サイズ取得
        screen_width = self.config.get("SCREEN_WIDTH", 800)
        screen_height = self.config.get("SCREEN_HEIGHT", 480)
        
        # グラデーション段数（多いほど滑らか）
        gradient_steps = 50
        step_height = screen_height / gradient_steps
        
        # ベース色から明るい色と暗い色を計算
        rgb_base = tuple(int(base_color[i:i+2], 16) for i in (1, 3, 5))
        
        for i in range(gradient_steps):
            # 上部は明るく、下部は暗く
            lightness_factor = 1.3 - (i / gradient_steps) * 0.6  # 1.3 -> 0.7
            
            # RGB値を調整（255を超えないように制限）
            rgb_adjusted = tuple(
                min(255, max(0, int(rgb_base[j] * lightness_factor)))
                for j in range(3)
            )
            
            color = f"#{rgb_adjusted[0]:02x}{rgb_adjusted[1]:02x}{rgb_adjusted[2]:02x}"
            
            # 矩形を描画
            y1 = i * step_height
            y2 = (i + 1) * step_height
            
            rect_id = self.canvas.create_rectangle(
                0, y1, screen_width, y2,
                fill=color, outline=color
            )
            self.background_gradient_items.append(rect_id)
        
        # 初回のみUI要素を前面に移動
        if len(self.background_gradient_items) == gradient_steps:
            self._bring_ui_elements_to_front()
    
    def _bring_ui_elements_to_front(self):
        """UI要素をグラデーション背景の前面に移動（初回のみ）"""
        # display_elementsの各要素を前面に移動
        for element_id in self.display_elements.values():
            if element_id:
                self.canvas.tag_raise(element_id)
    
    def _exit_fullscreen(self, event=None):
        """フルスクリーン終了"""
        if self.root:
            self.root.attributes('-fullscreen', False)
    
    def show_splash_screen(self):
        """スプラッシュ画面表示（フェードイン・アウト付き）"""
        if not self.canvas:
            return
            
        # 既存のスプラッシュがあれば削除
        self._clear_splash_elements()
        
        # 黒背景を作成
        self.splash_background_id = self.canvas.create_rectangle(
            0, 0,
            self.config["SCREEN_WIDTH"],
            self.config["SCREEN_HEIGHT"],
            fill="black",
            outline="black"
        )
        
        # スプラッシュテキストを作成（初期は透明）
        splash_font = self.config.get("FONTS", {}).get("splash", ("Arial", 32, "bold"))
        self.splash_text_id = self.canvas.create_text(
            self.config["SCREEN_WIDTH"] // 2,
            self.config["SCREEN_HEIGHT"] // 2,
            text="CarBuddy\nStarting...",
            fill="#000000",  # 初期は透明（黒）
            font=splash_font,
            justify="center"
        )
        
        # スプラッシュ要素を最前面に移動
        self.canvas.tag_raise(self.splash_background_id)
        self.canvas.tag_raise(self.splash_text_id)
        
        self.root.update()
        
        # フェードインアニメーション開始
        self.splash_fade_step = 0
        self._start_fade_in_animation()
    
    def _start_fade_in_animation(self):
        """フェードインアニメーション開始"""
        self._animate_fade_in()
    
    def _animate_fade_in(self):
        """フェードインアニメーションの1ステップ"""
        if self.splash_fade_step >= self.splash_fade_steps:
            # フェードイン完了、1秒待機してからフェードアウト開始
            self.root.after(1000, self._start_fade_out_animation)
            return
        
        # 透明度計算（0.0 〜 1.0）
        alpha = self.splash_fade_step / self.splash_fade_steps
        
        # 白色の透明度を計算（255 * alpha）
        color_value = int(255 * alpha)
        color = f"#{color_value:02x}{color_value:02x}{color_value:02x}"
        
        # テキストの色を更新
        if self.splash_text_id:
            self.canvas.itemconfig(self.splash_text_id, fill=color)
        
        # 次のステップをスケジュール
        self.splash_fade_step += 1
        step_delay = self.splash_fade_duration // self.splash_fade_steps
        self.splash_animation_timer = self.root.after(step_delay, self._animate_fade_in)
    
    def _start_fade_out_animation(self):
        """フェードアウトアニメーション開始"""
        self.splash_fade_step = self.splash_fade_steps
        self._animate_fade_out()
    
    def _animate_fade_out(self):
        """フェードアウトアニメーションの1ステップ"""
        if self.splash_fade_step <= 0:
            # フェードアウト完了、スプラッシュ削除
            self._hide_splash_screen()
            return
        
        # 透明度計算（1.0 〜 0.0）
        alpha = self.splash_fade_step / self.splash_fade_steps
        
        # 白色の透明度を計算
        color_value = int(255 * alpha)
        color = f"#{color_value:02x}{color_value:02x}{color_value:02x}"
        
        # テキストの色を更新
        if self.splash_text_id:
            self.canvas.itemconfig(self.splash_text_id, fill=color)
        
        # 次のステップをスケジュール
        self.splash_fade_step -= 1
        step_delay = self.splash_fade_duration // self.splash_fade_steps
        self.splash_animation_timer = self.root.after(step_delay, self._animate_fade_out)
    
    def _clear_splash_elements(self):
        """スプラッシュ要素をクリア"""
        if self.splash_animation_timer:
            self.root.after_cancel(self.splash_animation_timer)
            self.splash_animation_timer = None
            
        if self.splash_text_id:
            self.canvas.delete(self.splash_text_id)
            self.splash_text_id = None
            
        if self.splash_background_id:
            self.canvas.delete(self.splash_background_id)
            self.splash_background_id = None
    
    def _hide_splash_screen(self):
        """スプラッシュ画面を非表示してメインUIを描画"""
        # すべてのスプラッシュ要素をクリア
        self._clear_splash_elements()
            
        # スプラッシュ終了後にメインUIを描画
        self._draw_initial_ui()
    
    def run_main_loop(self):
        """メインループ開始"""
        if self.root:
            self.root.mainloop()
    
    def destroy(self):
        """リソース解放"""
        # アニメーションタイマーをキャンセル
        if self.animation_timer:
            self.root.after_cancel(self.animation_timer)
            self.animation_timer = None
            
        # スプラッシュアニメーションタイマーをキャンセル
        if self.splash_animation_timer:
            self.root.after_cancel(self.splash_animation_timer)
            self.splash_animation_timer = None
            
            
        if self.root:
            try:
                self.root.destroy()
            except Exception:
                pass  # 既に破棄されている場合は無視
            finally:
                self.root = None