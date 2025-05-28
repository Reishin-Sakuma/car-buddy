#include <Arduino.h>
#include <display.h>

void setup() {
  initDisplay();
}

void loop() {
  float temp = 36.2;  // 仮温度
  int speed = 42;     // 仮速度

  drawTemperature(temp);
  drawSpeed(speed);

  delay(2000);  // 2秒ごとに更新
}
