// #include <Arduino.h>
// #include <display.h>

// void setup() {
//   initDisplay();
// }

// void loop() {
//   float temp = 36.2;  // 仮温度
//   int speed = 42;     // 仮速度

//   drawTemperature(temp);
//   drawSpeed(speed);

//   delay(2000);  // 2秒ごとに更新
// }

#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("ESP32 起動完了！ようこそ『しゃべるモニター』へ！");
}

void loop() {
  Serial.println("動作中…");
  delay(1000);
}
