#include <Arduino.h>
#include "display.hpp"
#include "temp_sensor.hpp"

void setup() {
  Serial.begin(115200);
  initDisplay();
  initTempSensor();
}

void loop() {
  float tempC = readTemperature();
  displayTemperature(tempC);
  delay(2000);
}
