#include <OneWire.h>
#include <DallasTemperature.h>
#include "temperature.hpp"

#define ONE_WIRE_BUS 21

// 静的にスコープ内に持ち、setup後に初期化
static OneWire* oneWire = nullptr;
static DallasTemperature* sensors = nullptr;

void initTemperatureSensor() {
  oneWire = new OneWire(ONE_WIRE_BUS);
  sensors = new DallasTemperature(oneWire);
  sensors->begin();
}

float readTemperatureC() {
  if (sensors) {
    sensors->requestTemperatures();
    return sensors->getTempCByIndex(0);
  }
  return NAN;
}

float getTemperature() {
  return readTemperatureC();
}
