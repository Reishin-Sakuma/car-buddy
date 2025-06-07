#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "temperature.hpp"

TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(115200);
    Serial.println("Serial OK");

    tft.init();
    tft.setRotation(1);
    Serial.println("TFT init OK");

    tft.fillScreen(TFT_BLUE);  // 真っ白回避のためにテスト描画
    delay(500);

    drawUI(60.0);
    drawCharacterImage(10, 20);

    Serial.println("initTemperatureSensor: start");
    initTemperatureSensor();
    Serial.println("initTemperatureSensor: done");

    float temp = getTemperature();
    Serial.print("Temp = ");
    Serial.println(temp);
}

void loop() {
    float temp = getTemperature();
    Serial.print("Current Temp = ");
    Serial.println(temp);

    drawTemperature(temp);
    
    delay(1000);  // 1秒ごとに更新
}