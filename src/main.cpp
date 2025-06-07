#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "temperature.hpp"

TFT_eSPI tft = TFT_eSPI();

void setup() {
    tft.init();
    tft.setRotation(1);
    drawUI(25.5, 60.0);
    // キャラクター画像の描画（例として座標(10, 20)に描画）
    drawCharacterImage(10, 20);
    Serial.begin(115200);
    initTemperatureSensor();
}

void loop() {
    // 必要に応じてUI更新
  float temp = readTemperatureC();
  Serial.print("Temp: ");
  Serial.print(temp);
  Serial.println(" °C");

  delay(1000);
}
