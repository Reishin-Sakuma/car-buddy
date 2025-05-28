#include "display.h"
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();  // TFT_eSPI は内部で SPI.begin() を呼ぶ

void initDisplay() {
  tft.init();
  tft.setRotation(1);  // 横向き表示に設定（0=縦, 1=右回転, 2=上下反転, 3=左回転） 
  tft.fillScreen(TFT_BLACK); // 背景を黒に設定
  drawUILayout(); // レイアウトを描画
}

void drawUILayout() {
  // レイアウト枠線
  tft.drawRect(0, 0, 160, 240, TFT_WHITE);         // 左：キャラクター表示エリア
  tft.drawRect(160, 0, 160, 120, TFT_BLUE);        // 右上：温度表示エリア
  tft.drawRect(160, 120, 160, 120, TFT_GREEN);     // 右下：速度表示エリア
}

void drawTemperature(float temp) {  // 温度表示
  tft.setTextColor(TFT_WHITE, TFT_BLUE);  // テキスト色と背景色を設定
  tft.setTextSize(2); // テキストサイズを2倍に設定
  tft.setCursor(170, 30); // 表示位置を設定
  tft.fillRect(160, 0, 160, 120, TFT_BLUE);  // 表示更新前に領域を塗りつぶし
  tft.setCursor(170, 30); // テキストカーソル位置を設定
  tft.printf("温度: %.1f C", temp); // 温度を小数点以下1桁で表示
}

void drawSpeed(int spd) { // 速度表示
  tft.setTextColor(TFT_WHITE, TFT_GREEN); // テキスト色と背景色を設定
  tft.setTextSize(2); // テキストサイズを2倍に設定
  tft.fillRect(160, 120, 160, 120, TFT_GREEN);  // 表示更新前に領域を塗りつぶし
  tft.setCursor(170, 160);  // テキストカーソル位置を設定
  tft.printf("速度: %d km/h", spd); // 速度を整数で表示
}
