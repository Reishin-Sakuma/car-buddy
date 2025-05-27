#include <OneWire.h>
#include <DallasTemperature.h>
#include "temp_sensor.h"
#define TEMP_SENSOR_PIN 4
OneWire oneWire(TEMP_SENSOR_PIN);
DallasTemperature sensors(&oneWire);

void initTempSensor() {
  sensors.begin();
}

float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}