#include <OneWire.h>
#include <DallasTemperature.h>
#include "temperature.hpp"

// 接続しているGPIOピンに合わせて変更
#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void initTemperatureSensor() {
  sensors.begin();
}

float readTemperatureC() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}
