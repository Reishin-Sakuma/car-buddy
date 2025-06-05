#include <Arduino.h>
#include <TFT_eSPI.h>

// TFT_eSPI設定用のピン定義をカスタマイズ
// #define TFT_CS   5
// #define TFT_RST  4
// #define TFT_DC   2

TFT_eSPI tft = TFT_eSPI();  // TFTインスタンス生成

void setup() {
  Serial.begin(115200);
  delay(100);

  // TFT設定（CS, DC, RSTはTFT_eSPIのUser_Setupファイルで設定も可）
  // もしUser_Setupで設定してなければ、ここでピンを再設定する必要あり
  // ただしTFT_eSPIはUser_Setupファイルを使うのが基本なので要注意

  tft.init();
  tft.setRotation(1); // 横向き表示
  tft.fillScreen(TFT_BLACK);

  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(2);
  tft.drawString("Hello, car-buddy!", 10, 20);

  Serial.println("TFT表示テスト開始");
}

void loop() {
  // 特にループ処理なし。静止画表示だけ
}
