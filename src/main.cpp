#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "temperature.hpp"
#include "speed.hpp"

TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(115200);
    Serial.println("Serial OK");

    tft.init();
    tft.setRotation(1);
    Serial.println("TFT init OK");

    tft.fillScreen(TFT_BLUE);  // 真っ白回避のためにテスト描画
    delay(500);

    drawUI();
    drawCharacterImage(10, 20);

    Serial.println("initTemperatureSensor: start");
    initTemperatureSensor();
    Serial.println("initTemperatureSensor: done");

    Serial.println("initSpeedSensor: start");
    bool success = initSpeedSensor();
    Serial.println(success ? "Speed sensor init OK" : "Speed sensor init FAILED");

    float temp = getTemperature();
    Serial.print("Temp = ");
    Serial.println(temp);
}

void loop() {
    float temp = getTemperature();
    drawTemperature(temp);

    float speed = getSpeed();  // 実際は加速度
    drawSpeed(speed);          // UI表示

    Serial.print("Speed = ");
    Serial.println(speed);
    Serial.print("Temperature = ");
    Serial.println(temp);

    delay(1000);
}