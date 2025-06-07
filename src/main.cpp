#include <Arduino.h>
#include <Wire.h>

#define MPU_ADDR 0x68

void setup() {
  Serial.begin(115200);
  Wire.begin();
}

void loop() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x3B);  // 加速度計のX軸データ開始アドレス
  Wire.endTransmission(false);

  Wire.requestFrom((uint8_t)MPU_ADDR, (size_t)6, true);

  if (Wire.available() >= 6) {
    int16_t accX = (Wire.read() << 8) | Wire.read();
    int16_t accY = (Wire.read() << 8) | Wire.read();
    int16_t accZ = (Wire.read() << 8) | Wire.read();

    Serial.print("AccX: "); Serial.print(accX);
    Serial.print(" AccY: "); Serial.print(accY);
    Serial.print(" AccZ: "); Serial.println(accZ);
  }

  delay(500);
}
