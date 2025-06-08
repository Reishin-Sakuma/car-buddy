#include <OneWire.h>
#include <DallasTemperature.h>
#include "temperature.hpp"

#define ONE_WIRE_BUS 25
#define TEMP_PRECISION 12  // 12ビット精度（0.0625℃単位）

// staticインスタンスでメモリリーク回避
static OneWire oneWire(ONE_WIRE_BUS);
static DallasTemperature sensors(&oneWire);
static bool sensorReady = false;
static unsigned long lastReadTime = 0;
static float lastValidTemp = 20.0;  // デフォルト値

void initTemperatureSensor() {
  Serial.println("Initializing temperature sensor...");
  
  sensors.begin();
  
  // センサー数を確認
  int deviceCount = sensors.getDeviceCount();
  Serial.print("Found ");
  Serial.print(deviceCount);
  Serial.println(" temperature sensor(s)");
  
  if (deviceCount > 0) {
    // 精度設定
    sensors.setResolution(TEMP_PRECISION);
    sensorReady = true;
    Serial.println("Temperature sensor ready!");
  } else {
    Serial.println("No temperature sensor found!");
    sensorReady = false;
  }
}

float readTemperatureC() {
  if (!sensorReady) {
    Serial.println("Temperature sensor not ready");
    return lastValidTemp;
  }
  
  // 750ms以内の連続読み取りを防ぐ（DS18B20の変換時間）
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime < 750) {
    return lastValidTemp;
  }
  
  sensors.requestTemperatures();
  float temp = sensors.getTempCByIndex(0);
  
  // 有効な値かチェック
  if (temp != DEVICE_DISCONNECTED_C && temp > -55.0 && temp < 125.0) {
    lastValidTemp = temp;
    lastReadTime = currentTime;
    return temp;
  } else {
    Serial.println("Invalid temperature reading");
    return lastValidTemp;  // 前回の有効値を返す
  }
}

float getTemperature() {
  return readTemperatureC();
}