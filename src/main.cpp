#include <Arduino.h>
#include <TFT_eSPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SdFat.h>
#include <FS.h>
#include <SPI.h>

#define TEMP_SENSOR_PIN 4
#define SD_CS_PIN 5
#define SPEAKER_PIN 25

TFT_eSPI tft = TFT_eSPI();
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);
SdFat SD;

void setup() {
  Serial.begin(115200);
  delay(100);

  // LCD初期化
  tft.init();
  tft.setRotation(1); // 横向き
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE);
  tft.drawString("car-buddy 起動中...", 10, 10);

  // 温度センサ初期化
  sensors.begin();

  // SDカード初期化
  if (!SD.begin(SD_CS_PIN)) {
    tft.drawString("SDカード未検出", 10, 30);
  } else {
    tft.drawString("SDカードOK", 10, 30);
  }
}

void loop() {
  sensors.requestTemperatures();
  float tempC = sensors.getTempCByIndex(0);

  tft.fillRect(10, 60, 200, 30, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("温度: " + String(tempC, 1) + " C", 10, 60);

  delay(2000);
}

// 今後の追加: WAV音声再生処理（ESP32用ライブラリ選定次第）