#include <OneWire.h>
#include <DallasTemperature.h>
#include "temp_sensor.h"

#define ONE_WIRE_BUS 4

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void initTempSensor() {
  sensors.begin();
}

float readTemperature() {
  sensors.requestTemperatures();
  return sensors.getTempCByIndex(0);
}
