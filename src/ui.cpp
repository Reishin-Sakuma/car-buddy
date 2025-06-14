#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characterImage.h"
#include "../include/temperature.hpp"
#include "../include/speed.hpp"
#include "../include/time.hpp"

extern TFT_eSPI tft;

// 状態管理変数
static bool uiInitialized = false;
static float lastTemperature = -999.0;
static float lastSpeed = -999.0;
static String lastTime = "";
static String lastDate = "";
static bool characterDisplayed = false;

// 温度連動背景色用の変数
static float currentBackgroundTemp = 20.0;  // デフォルト温度
static float lastBackgroundUpdateTemp = -999.0;

// === 温度連動グラデーション色計算 ===

// 温度に応じた色を計算（高温時の緑色問題を完全修正）
void getTemperatureColors(float temp, uint8_t* topR, uint8_t* topG, uint8_t* topB, 
                         uint8_t* bottomR, uint8_t* bottomG, uint8_t* bottomB) {
    if (temp >= 32.0) {
        // 32℃以上：確実に赤色系グラデーション（濃い赤 → 明るい赤）
        *topR = 120;   *topG = 0;     *topB = 0;      // 上部：濃い赤
        *bottomR = 255; *bottomG = 60;  *bottomB = 60;   // 下部：明るい赤
    } else if (temp >= 30.0) {
        // 30-32℃：青から赤への確実な遷移（緑を完全に避ける）
        float ratio = (temp - 30.0) / 2.0;  // 0.0 → 1.0
        
        // 青色から赤色への直接補間（緑成分を最小限に）
        *topR = (uint8_t)(0 + (120 * ratio));      // 0 → 120
        *topG = (uint8_t)(0 + (0 * ratio));       // 0 → 0 (緑成分なし)
        *topB = (uint8_t)(128 * (1.0 - ratio));   // 128 → 0
        
        *bottomR = (uint8_t)(64 + (191 * ratio));  // 64 → 255
        *bottomG = (uint8_t)(128 * (1.0 - ratio) + (60 * ratio)); // 128 → 60 (緑成分を抑制)
        *bottomB = (uint8_t)(255 * (1.0 - ratio) + (60 * ratio)); // 255 → 60
    } else if (temp >= 25.0) {
        // 25-30℃：青系グラデーション
        *topR = 0; *topG = 20; *topB = 100;     
        *bottomR = 40; *bottomG = 80; *bottomB = 180;  
    } else {
        // 25℃未満：濃い青色グラデーション
        *topR = 0; *topG = 0; *topB = 128;    
        *bottomR = 64; *bottomG = 128; *bottomB = 255;
    }
}

// 温度連動グラデーション背景を描画
void drawTemperatureGradientBackground(float temp) {
    uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
    getTemperatureColors(temp, &topR, &topG, &topB, &bottomR, &bottomG, &bottomB);
    
    for (int y = 0; y < 240; y++) {
        // グラデーション計算（0.0から1.0の範囲）
        float ratio = (float)y / 240.0;
        
        // 上部色から下部色への補間
        uint8_t r = (uint8_t)(topR + ((bottomR - topR) * ratio));
        uint8_t g = (uint8_t)(topG + ((bottomG - topG) * ratio));
        uint8_t b = (uint8_t)(topB + ((bottomB - topB) * ratio));
        
        uint16_t lineColor = tft.color565(r, g, b);
        
        // 横一列を描画
        tft.drawFastHLine(0, y, 320, lineColor);
    }
}

// 特定エリアの温度連動グラデーション背景を再描画（文字消去用）
void drawTemperatureGradientArea(int x, int y, int width, int height, float temp) {
    uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
    getTemperatureColors(temp, &topR, &topG, &topB, &bottomR, &bottomG, &bottomB);
    
    for (int row = y; row < y + height; row++) {
        if (row >= 240) break;  // 画面外チェック
        
        float ratio = (float)row / 240.0;
        uint8_t r = (uint8_t)(topR + ((bottomR - topR) * ratio));
        uint8_t g = (uint8_t)(topG + ((bottomG - topG) * ratio));
        uint8_t b = (uint8_t)(topB + ((bottomB - topB) * ratio));
        uint16_t lineColor = tft.color565(r, g, b);
        
        tft.drawFastHLine(x, row, width, lineColor);
    }
}

// === 既存関数を温度連動に変更 ===

// 基本的なグラデーション背景を描画（後方互換性のため残す）
void drawGradientBackground() {
    drawTemperatureGradientBackground(currentBackgroundTemp);
}

// 特定エリアのグラデーション背景を再描画（後方互換性のため残す）
void drawGradientArea(int x, int y, int width, int height) {
    drawTemperatureGradientArea(x, y, width, height, currentBackgroundTemp);
}

// 背景色の温度を更新（他のモジュールから呼び出される）
void updateBackgroundTemperature(float temp) {
    currentBackgroundTemp = temp;
}

// ===== CarBuddyタイトル表示関数 =====

// 画面上部のキャラクター空白部分にCarBuddyタイトルを表示
void drawCarBuddyTitle() {
    // タイトル設定（キャラクター画像の上部空白に配置）
    String title = "CarBuddy";
    int titleSize = 2;  // フォントサイズ
    int titleX = 25;    // キャラクター画像の上部中央に配置
    int titleY = 8;     // キャラクター画像の上部空白部分
    
    // タイトル用の背景エリアを温度連動グラデーションで描画
    drawTemperatureGradientArea(titleX - 2, titleY - 2, 
                               title.length() * 12 + 4, 20, currentBackgroundTemp);
    
    // タイトルテキストを白色で描画（視認性重視）
    tft.setTextSize(titleSize);
    tft.setTextColor(TFT_WHITE);
    tft.drawString(title, titleX, titleY);
}

// タイトルエリアの更新（温度変化時の再描画用）
void updateCarBuddyTitle() {
    drawCarBuddyTitle();
}

// ===== 既存関数（温度連動背景対応版） =====

// スプラッシュ画面を表示
void showSplashScreen() {
    // 基本設定
    String title = "car-buddy";
    String subtitle = "Talking Monitor v1.0";
    int titleX = (320 - title.length() * 24) / 2;
    int titleY = (240 - 32) / 2;
    int subX = (320 - subtitle.length() * 12) / 2;
    int subY = titleY + 50;
    
    // フェードイン（16段階でより滑らか）
    for (int fade = 0; fade <= 15; fade++) {
        tft.fillScreen(TFT_BLACK);
        
        // より広い範囲でフェード効果（0-255の範囲を使用）
        uint8_t brightness = fade * 17;  // 0, 17, 34, ..., 255
        uint16_t fadeColor = tft.color565(brightness, brightness, brightness);
        tft.setTextColor(fadeColor, TFT_BLACK);
        
        // タイトル描画
        tft.setTextSize(4);
        tft.drawString(title, titleX, titleY);
        
        // サブタイトル描画
        tft.setTextSize(2);
        tft.drawString(subtitle, subX, subY);
        
        delay(150);  // フェード速度を少し遅く
    }
    
    // 完全表示（白色で確実に表示）
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(4);
    tft.drawString(title, titleX, titleY);
    tft.setTextSize(2);
    tft.drawString(subtitle, subX, subY);
    
    delay(1500);  // 完全表示時間を少し長く
    
    // フェードアウト（16段階）
    for (int fade = 15; fade >= 0; fade--) {
        tft.fillScreen(TFT_BLACK);
        
        // フェードアウト効果
        uint8_t brightness = fade * 17;
        uint16_t fadeColor = tft.color565(brightness, brightness, brightness);
        tft.setTextColor(fadeColor, TFT_BLACK);
        
        // タイトル描画
        tft.setTextSize(4);
        tft.drawString(title, titleX, titleY);
        
        // サブタイトル描画
        tft.setTextSize(2);
        tft.drawString(subtitle, subX, subY);
        
        delay(120);  // フェードアウト速度
    }
    
    // 最終的に黒画面
    tft.fillScreen(TFT_BLACK);
    delay(300);
}

void drawUI() {
    if (!uiInitialized) {
        // メイン画面のフェードイン
        fadeInMainScreen();
        uiInitialized = true;
    }
}

// メイン画面をフェードインで表示（元の位置に戻す）
void fadeInMainScreen() {
    // フェードイン（8段階）
    for (int fade = 0; fade <= 7; fade++) {
        // 温度連動グラデーション背景を描画
        drawTemperatureGradientBackground(currentBackgroundTemp);
        
        // フェード効果のためのオーバーレイ
        if (fade < 7) {
            uint8_t fadeAlpha = 255 - (fade * 32);
            uint16_t fadeOverlay = tft.color565(fadeAlpha / 8, fadeAlpha / 8, fadeAlpha / 4);
            
            // 半透明オーバーレイでフェード効果（軽量化版）
            for (int y = 0; y < 240; y += 3) {
                for (int x = 0; x < 320; x += 3) {
                    if ((x + y) % 6 == 0) {  // まばらなパターンで軽量化
                        tft.drawPixel(x, y, fadeOverlay);
                    }
                }
            }
        }
        
        // テキスト色のフェード（白文字で視認性重視）
        uint16_t textColor = tft.color565(fade * 32, fade * 32, fade * 32);
        tft.setTextColor(textColor);
        
        // CarBuddyタイトルを表示（キャラクター上部空白に）
        if (fade >= 3) {  // 少し遅れてタイトル表示
            tft.setTextSize(2);
            tft.drawString("CarBuddy", 25, 8);  // キャラクター上部空白に配置
        }
        
        // 固定ラベルを描画（元の位置に戻す）
        tft.setTextSize(3);
        tft.drawString("Temp:", 200, 10);     // 元の位置に戻す
        tft.drawString("Speed:", 200, 130);
        tft.drawString("km/h", 240, 180);     // 元の位置に戻す
        
        delay(80);  // フェード速度
    }
    
    // 最終的に完全な温度連動グラデーション背景で描画
    drawTemperatureGradientBackground(currentBackgroundTemp);
    
    // 最終テキスト描画（白文字で見やすく）
    tft.setTextColor(TFT_WHITE);
    
    // CarBuddyタイトルを表示（キャラクター上部空白に）
    tft.setTextSize(2);
    tft.drawString("CarBuddy", 25, 8);
    
    // 固定ラベル（元の位置）
    tft.setTextSize(3);
    tft.drawString("Temp:", 200, 10);
    tft.drawString("Speed:", 200, 130);
    tft.drawString("km/h", 240, 180);
}

// キャラクター画像をフェードインで表示
void drawCharacterImageWithFade(int x, int y) {
    const int originalSize = 160;
    const int newSize = 180;
    const float scale = (float)newSize / originalSize;
    
    // フェードイン（8段階）
    for (int fade = 0; fade <= 7; fade++) {
        tft.startWrite();
        tft.setAddrWindow(x, y, newSize, newSize);
        
        for (int row = 0; row < newSize; row++) {
            for (int col = 0; col < newSize; col++) {
                int srcRow = (int)(row / scale);
                int srcCol = (int)(col / scale);
                
                if (srcRow >= originalSize) srcRow = originalSize - 1;
                if (srcCol >= originalSize) srcCol = originalSize - 1;
                
                int srcIndex = srcRow * originalSize + srcCol;
                uint16_t originalColor = pgm_read_word(&characterImage[srcIndex]);
                
                // 色をフェード処理
                uint8_t r = ((originalColor >> 11) & 0x1F) * fade / 7;
                uint8_t g = ((originalColor >> 5) & 0x3F) * fade / 7;
                uint8_t b = (originalColor & 0x1F) * fade / 7;
                uint16_t fadeColor = tft.color565(r << 3, g << 2, b << 3);
                
                tft.pushColor(fadeColor);
            }
        }
        
        tft.endWrite();
        delay(60);  // キャラクターフェード速度
    }
}

// 通常のキャラクター画像表示（フェードなし）
void drawCharacterImage(int x, int y) {
    const int originalSize = 160;
    const int newSize = 180;
    const float scale = (float)newSize / originalSize;
    
    tft.startWrite();
    tft.setAddrWindow(x, y, newSize, newSize);
    
    for (int row = 0; row < newSize; row++) {
        for (int col = 0; col < newSize; col++) {
            int srcRow = (int)(row / scale);
            int srcCol = (int)(col / scale);
            
            if (srcRow >= originalSize) srcRow = originalSize - 1;
            if (srcCol >= originalSize) srcCol = originalSize - 1;
            
            int srcIndex = srcRow * originalSize + srcCol;
            uint16_t color = pgm_read_word(&characterImage[srcIndex]);
            
            tft.pushColor(color);
        }
    }
    
    tft.endWrite();
}

// キャラクター画像を縁ぼかし効果付きで表示
void drawCharacterImageWithEdgeFade(int x, int y) {
    const int originalSize = 160;
    const int newSize = 180;
    const float scale = (float)newSize / originalSize;
    const int fadeWidth = 8;  // フェード幅（ピクセル）
    
    tft.startWrite();
    tft.setAddrWindow(x, y, newSize, newSize);
    
    for (int row = 0; row < newSize; row++) {
        for (int col = 0; col < newSize; col++) {
            int srcRow = (int)(row / scale);
            int srcCol = (int)(col / scale);
            
            if (srcRow >= originalSize) srcRow = originalSize - 1;
            if (srcCol >= originalSize) srcCol = originalSize - 1;
            
            int srcIndex = srcRow * originalSize + srcCol;
            uint16_t originalColor = pgm_read_word(&characterImage[srcIndex]);
            
            // 縁からの距離を計算
            int distanceFromEdge = min(min(row, newSize - row - 1), min(col, newSize - col - 1));
            
            if (distanceFromEdge < fadeWidth) {
                // フェード処理
                float alpha = 0.3 + 0.7 * ((float)distanceFromEdge / fadeWidth);
                
                // 背景色を取得（温度連動グラデーション）
                float backgroundRatio = (float)(y + row) / 240.0;
                uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
                getTemperatureColors(currentBackgroundTemp, &topR, &topG, &topB, &bottomR, &bottomG, &bottomB);
                
                uint8_t bgR = (uint8_t)(topR + ((bottomR - topR) * backgroundRatio));
                uint8_t bgG = (uint8_t)(topG + ((bottomG - topG) * backgroundRatio));
                uint8_t bgB = (uint8_t)(topB + ((bottomB - topB) * backgroundRatio));
                
                // 色の分解と合成
                uint8_t charR = (originalColor >> 11) & 0x1F;
                uint8_t charG = (originalColor >> 5) & 0x3F;
                uint8_t charB = originalColor & 0x1F;
                
                // アルファブレンド
                uint8_t finalR = (uint8_t)((charR << 3) * alpha + bgR * (1.0 - alpha)) >> 3;
                uint8_t finalG = (uint8_t)((charG << 2) * alpha + bgG * (1.0 - alpha)) >> 2;
                uint8_t finalB = (uint8_t)((charB << 3) * alpha + bgB * (1.0 - alpha)) >> 3;
                
                uint16_t finalColor = tft.color565(finalR << 3, finalG << 2, finalB << 3);
                tft.pushColor(finalColor);
            } else {
                // フェードなし
                tft.pushColor(originalColor);
            }
        }
    }
    
    tft.endWrite();
}

// ===== 差分描画対応の表示関数 =====

// 温度表示（元の位置に戻す）
void drawTemperature(float temp) {
    // 値が変わった時のみ更新
    if (abs(temp - lastTemperature) > 0.1) {
        // 背景の温度連動グラデーション色を再描画（温度表示エリア + タイトルエリア）
        drawTemperatureGradientArea(195, 30, 130, 35, currentBackgroundTemp);
        
        // CarBuddyタイトルも更新（背景色変化に対応）
        updateCarBuddyTitle();
        
        // 温度値による文字色の判定
        uint16_t tempTextColor = TFT_WHITE;
        if (temp >= 32.0) {
            tempTextColor = TFT_YELLOW;  // 高温時は警告として黄色
        }
        
        // フォントサイズを明示的に設定
        tft.setTextSize(3);
        tft.setTextColor(tempTextColor);
        tft.drawString(String(temp, 1) + " C", 200, 35);  // 元の位置に戻す
        lastTemperature = temp;
    }
}

// 全体同期表示（元の位置に戻す）
void forceFullRedraw(float temp) {
    // 背景が大きく変わった時の処理
    if (abs(temp - lastBackgroundUpdateTemp) > 1.0) {
        Serial.println("Background temperature change detected - forcing full redraw");
        
        // 1. 現在値を取得
        float currentSpeed = getSpeed();
        String currentTimeStr = getCurrentTime();
        String currentDateStr = getCurrentDate();
        
        // 2. 背景を更新
        drawTemperatureGradientBackground(temp);
        lastBackgroundUpdateTemp = temp;
        
        // 3. すべての要素を即座に描画
        
        // CarBuddyタイトル（キャラクター上部空白に）
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(2);
        tft.drawString("CarBuddy", 25, 8);
        
        // UI固定ラベル（元の位置に戻す）
        tft.setTextSize(3);
        tft.drawString("Temp:", 200, 10);
        tft.drawString("Speed:", 200, 130);
        tft.drawString("km/h", 240, 180);
        
        // 温度表示（文字色判定付き）
        uint16_t tempTextColor = TFT_WHITE;
        if (temp >= 32.0) {
            tempTextColor = TFT_YELLOW;
        }
        tft.setTextSize(3);
        tft.setTextColor(tempTextColor);
        tft.drawString(String(temp, 1) + " C", 200, 35);
        
        // 速度表示
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE);
        tft.drawString(String(abs(currentSpeed), 1), 200, 155);
        
        // 時刻表示
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW);
        tft.drawString(currentTimeStr, 10, 220);
        
        // 日付表示
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN);
        tft.drawString(currentDateStr, 100, 220);
        
        // キャラクター描画（元の位置）
        drawCharacterImageWithEdgeFade(10, 30);  // 元の位置に戻す
        
        // 前回値を更新
        forceUpdateAllDisplayValues();
        setLastDisplayValues(temp, currentSpeed, currentTimeStr, currentDateStr);
    }
}

// 速度表示（元の位置に戻す）
void drawSpeed(float speed) {
    // 値が変わった時のみ更新
    if (abs(speed - lastSpeed) > 0.1) {
        // 背景の温度連動グラデーション色を再描画（速度表示エリア + 余裕をもって広めに）
        drawTemperatureGradientArea(195, 150, 130, 35, currentBackgroundTemp);
        
        // フォントサイズを明示的に設定
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE);
        tft.drawString(String(abs(speed), 1), 200, 155);  // 元の位置に戻す
        lastSpeed = speed;
    }
}

// 時刻表示（温度連動グラデーション背景対応）
void drawTime(String timeStr) {
    if (timeStr != lastTime) {
        // 背景の温度連動グラデーション色を再描画（時刻表示エリア）
        drawTemperatureGradientArea(5, 215, 100, 25, currentBackgroundTemp);
        
        // フォントサイズを明示的に設定
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW);
        tft.drawString(timeStr, 10, 220);
        lastTime = timeStr;
    }
}

// 日付表示（温度連動グラデーション背景対応）
void drawDate(String dateStr) {
    if (dateStr != lastDate) {
        // 背景の温度連動グラデーション色を再描画（日付表示エリア）
        drawTemperatureGradientArea(95, 215, 120, 25, currentBackgroundTemp);
        
        // フォントサイズを明示的に設定
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN);
        tft.drawString(dateStr, 100, 220);
        lastDate = dateStr;
    }
}

// キャラクター表示管理（元の位置）
void drawCharacter() {
    if (!characterDisplayed) {
        drawCharacterImageWithEdgeFade(10, 30);  // 元の位置に戻す
        characterDisplayed = true;
    }
}

// ===== 状態管理関数 =====

// 前回値を強制更新（全体再描画後に使用）
void forceUpdateAllDisplayValues() {
    lastTemperature = -999.0;
    lastSpeed = -999.0;
    lastTime = "";
    lastDate = "";
    characterDisplayed = false;
}

// 前回値を設定（強制再描画後に使用）
void setLastDisplayValues(float temp, float speed, String timeStr, String dateStr) {
    lastTemperature = temp;
    lastSpeed = speed;
    lastTime = timeStr;
    lastDate = dateStr;
    characterDisplayed = true;
}