#include <TFT_eSPI.h>
#include "ui.hpp"
#include "characterImage.h"
#include <pgmspace.h>

extern TFT_eSPI tft;

// 前回の値を保存してちらつき防止
static float lastTemp = -999.0;
static float lastSpeed = -999.0;
static bool uiInitialized = false;
static float currentBackgroundTemp = 20.0;  // 背景色計算用の現在温度

// 時刻表示用の静的変数
static String lastDisplayTime = "";
static String lastDisplayDate = "";

// 強制更新用の関数（背景変更時に使用）
void forceUpdateAllDisplayValues() {
    lastTemp = -999.0;
    lastSpeed = -999.0;
    lastDisplayTime = "";
    lastDisplayDate = "";
}

// テスト用：強制的に青背景を設定（デバッグ用）
void forceBlueBackground() {
    Serial.println("Force setting blue background...");
    currentBackgroundTemp = 20.0;  // 30℃未満の値を設定
    drawTemperatureGradientBackground(20.0);
    
    // UI要素を再描画
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(3);
    tft.drawString("Temp:", 200, 10);
    tft.drawString("Speed:", 200, 130);
    tft.drawString("km/h", 240, 180);
    
    // キャラクターも再描画
    drawCharacterImageWithEdgeFade(10, 30);
}

// ===== 温度連動グラデーション関数 =====

// 温度に基づいて背景色を決定する関数
void getTemperatureColors(float temp, uint8_t* topR, uint8_t* topG, uint8_t* topB, 
                         uint8_t* bottomR, uint8_t* bottomG, uint8_t* bottomB) {
    if (temp >= 32.0) {
        // 32℃以上：赤色系グラデーション（濃い赤 → 明るい赤）
        *topR = 80;    *topG = 0;     *topB = 0;      // 上部：濃い赤
        *bottomR = 255; *bottomG = 80;  *bottomB = 80;   // 下部：明るい赤
    } else if (temp >= 30.0) {
        // 30-32℃：青から赤への遷移
        float ratio = (temp - 30.0) / 2.0;  // 0.0 → 1.0
        
        // 青色から赤色への補間
        *topR = (uint8_t)(0 + (80 * ratio));       // 0 → 80
        *topG = (uint8_t)(0 * (1.0 - ratio));     // 0 → 0  
        *topB = (uint8_t)(128 * (1.0 - ratio));   // 128 → 0
        
        *bottomR = (uint8_t)(64 + (191 * ratio));  // 64 → 255
        *bottomG = (uint8_t)(128 * (1.0 - ratio) + (80 * ratio)); // 128 → 80
        *bottomB = (uint8_t)(255 * (1.0 - ratio) + (80 * ratio)); // 255 → 80
    } else {
        // 30℃未満：通常の青色グラデーション（濃い青 → 明るい青）
        *topR = 0;     *topG = 0;     *topB = 128;    // 上部：濃い青
        *bottomR = 64; *bottomG = 128; *bottomB = 255; // 下部：明るい青
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

// メイン画面をフェードインで表示（温度連動グラデーション対応）
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
        tft.setTextSize(3);
        
        // 固定ラベルを描画
        tft.drawString("Temp:", 200, 10);
        tft.drawString("Speed:", 200, 130);
        tft.drawString("km/h", 240, 180);
        
        delay(80);  // フェード速度
    }
    
    // 最終的に完全な温度連動グラデーション背景で描画
    drawTemperatureGradientBackground(currentBackgroundTemp);
    
    // 最終テキスト描画（白文字で見やすく）
    tft.setTextColor(TFT_WHITE);
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

// キャラクター画像を縁フェード付きで表示（温度連動グラデーション背景対応）
void drawCharacterImageWithEdgeFade(int x, int y) {
    const int originalSize = 160;
    const int newSize = 180;
    const float scale = (float)newSize / originalSize;
    const int fadeWidth = 8;  // フェード幅を小さく（弱め）
    
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
            
            // 四角い縁からの距離を計算
            int distFromEdge = min(min(row, col), min(newSize - 1 - row, newSize - 1 - col));
            
            if (distFromEdge < fadeWidth) {
                // フェード領域：温度連動グラデーション背景色と軽くブレンド
                float alpha = 0.3 + 0.7 * ((float)distFromEdge / fadeWidth);  // 0.3-1.0の範囲（弱め）
                
                // 元の色を分解
                uint8_t r = (originalColor >> 11) & 0x1F;
                uint8_t g = (originalColor >> 5) & 0x3F;
                uint8_t b = originalColor & 0x1F;
                
                // 温度連動グラデーション背景色を計算（その位置での背景色）
                uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
                getTemperatureColors(currentBackgroundTemp, &topR, &topG, &topB, &bottomR, &bottomG, &bottomB);
                
                float bgY = (float)(y + row) / 240.0;
                uint8_t bgR = (uint8_t)(topR + ((bottomR - topR) * bgY)) >> 3;  // 5ビットに変換
                uint8_t bgG = (uint8_t)(topG + ((bottomG - topG) * bgY)) >> 2; // 6ビットに変換
                uint8_t bgB = (uint8_t)(topB + ((bottomB - topB) * bgY)) >> 3; // 5ビットに変換
                
                // 軽いアルファブレンド
                uint8_t blendR = (uint8_t)(r * alpha + bgR * (1.0 - alpha));
                uint8_t blendG = (uint8_t)(g * alpha + bgG * (1.0 - alpha));
                uint8_t blendB = (uint8_t)(b * alpha + bgB * (1.0 - alpha));
                
                uint16_t fadeColor = (blendR << 11) | (blendG << 5) | blendB;
                tft.pushColor(fadeColor);
            } else {
                // 通常領域：元の色をそのまま
                tft.pushColor(originalColor);
            }
        }
    }
    
    tft.endWrite();
}

// より高品質な縁フェード（ガウシアンブラー風・温度連動対応）
void drawCharacterImageWithSoftEdge(int x, int y) {
    const int originalSize = 160;
    const int newSize = 180;
    const float scale = (float)newSize / originalSize;
    const int fadeWidth = 20;  // フェード幅
    
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
            
            // 中心からの距離でソフトな円形フェード
            float centerX = newSize / 2.0;
            float centerY = newSize / 2.0;
            float distFromCenter = sqrt((row - centerY) * (row - centerY) + (col - centerX) * (col - centerX));
            float maxRadius = newSize / 2.0;
            float fadeRadius = maxRadius - fadeWidth;
            
            if (distFromCenter > fadeRadius) {
                // フェード領域：円形グラデーション（温度連動背景対応）
                float alpha = 1.0 - (distFromCenter - fadeRadius) / fadeWidth;
                if (alpha < 0) alpha = 0;
                
                // 元の色を分解
                uint8_t r = (originalColor >> 11) & 0x1F;
                uint8_t g = (originalColor >> 5) & 0x3F;
                uint8_t b = originalColor & 0x1F;
                
                // 温度連動グラデーション背景色と自然にブレンド
                uint8_t topR, topG, topB, bottomR, bottomG, bottomB;
                getTemperatureColors(currentBackgroundTemp, &topR, &topG, &topB, &bottomR, &bottomG, &bottomB);
                
                float bgY = (float)(y + row) / 240.0;
                uint8_t bgR = (uint8_t)(topR + ((bottomR - topR) * bgY)) >> 3;
                uint8_t bgG = (uint8_t)(topG + ((bottomG - topG) * bgY)) >> 2;
                uint8_t bgB = (uint8_t)(topB + ((bottomB - topB) * bgY)) >> 3;
                
                uint8_t blendR = (uint8_t)(r * alpha + bgR * (1.0 - alpha));
                uint8_t blendG = (uint8_t)(g * alpha + bgG * (1.0 - alpha));
                uint8_t blendB = (uint8_t)(b * alpha + bgB * (1.0 - alpha));
                
                uint16_t fadeColor = (blendR << 11) | (blendG << 5) | blendB;
                tft.pushColor(fadeColor);
            } else {
                // 通常領域
                tft.pushColor(originalColor);
            }
        }
    }
    
    tft.endWrite();
}

// 時刻表示（温度連動グラデーション背景対応）
void drawTime(String currentTime) {
    // 時刻が変わった時のみ更新
    if (currentTime != lastDisplayTime) {
        // 背景の温度連動グラデーション色を再描画（時刻表示エリア + 余裕をもって広めに）
        drawTemperatureGradientArea(5, 215, 90, 30, currentBackgroundTemp);
        
        // 新しい時刻を描画
        tft.setTextSize(2);
        tft.setTextColor(TFT_YELLOW);
        tft.drawString(currentTime, 10, 220);
        lastDisplayTime = currentTime;
    }
}

// 日付表示（温度連動グラデーション背景対応）
void drawDate(String currentDate) {
    // 日付が変わった時のみ更新  
    if (currentDate != lastDisplayDate) {
        // 背景の温度連動グラデーション色を再描画（日付表示エリア + 余裕をもって広めに）
        drawTemperatureGradientArea(95, 215, 130, 30, currentBackgroundTemp);
        
        // 新しい日付を描画
        tft.setTextSize(2);
        tft.setTextColor(TFT_CYAN);
        tft.drawString(currentDate, 100, 220);
        lastDisplayDate = currentDate;
    }
}

// 温度表示（温度連動グラデーション背景対応）
void drawTemperature(float temp) {
    // 値が変わった時のみ更新
    if (abs(temp - lastTemp) > 0.1) {
        // 背景温度を更新（重要：この処理で背景色が変わる）
        updateBackgroundTemperature(temp);
        
        // 背景の温度連動グラデーション色を再描画（温度表示エリア + 余裕をもって広めに）
        drawTemperatureGradientArea(195, 30, 130, 35, temp);
        
        // 温度値によって文字色を変更（視認性向上）
        uint16_t textColor = TFT_WHITE;  // デフォルト：白
        if (temp >= 32.0) {
            textColor = TFT_YELLOW;  // 高温：黄色で警告
        }
        
        // フォントサイズを明示的に設定
        tft.setTextSize(3);
        tft.setTextColor(textColor);
        tft.drawString(String(temp, 1) + " C", 200, 35);
        lastTemp = temp;
        
        // 重要：温度変化時は常に全画面背景を更新（閾値を下げて確実に更新）
        static float lastBackgroundUpdateTemp = -999.0;  // 初期値を-999に設定
        if (abs(temp - lastBackgroundUpdateTemp) > 0.5) {  // 0.5℃変化で更新
            Serial.print("Background update triggered: ");
            Serial.print(lastBackgroundUpdateTemp);
            Serial.print("°C → ");
            Serial.print(temp);
            Serial.println("°C");
            
            drawTemperatureGradientBackground(temp);
            lastBackgroundUpdateTemp = temp;
            
            // UI要素を再描画
            tft.setTextColor(TFT_WHITE);
            tft.setTextSize(3);
            tft.drawString("Temp:", 200, 10);
            tft.drawString("Speed:", 200, 130);
            tft.drawString("km/h", 240, 180);
            
            // キャラクターも再描画
            drawCharacterImageWithEdgeFade(10, 30);
        }
    }
}

// 速度表示（温度連動グラデーション背景対応）
void drawSpeed(float speed) {
    // 値が変わった時のみ更新
    if (abs(speed - lastSpeed) > 0.1) {
        // 背景の温度連動グラデーション色を再描画（速度表示エリア + 余裕をもって広めに）
        drawTemperatureGradientArea(195, 150, 130, 35, currentBackgroundTemp);
        
        // フォントサイズを明示的に設定
        tft.setTextSize(3);
        tft.setTextColor(TFT_WHITE);
        tft.drawString(String(abs(speed), 1), 200, 155);
        lastSpeed = speed;
    }
}