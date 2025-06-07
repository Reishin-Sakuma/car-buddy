#include <Arduino.h>
#include <Wire.h>

// MPU6500のレジスタ定義
#define MPU6500_ADDRESS 0x68
#define WHO_AM_I_REG 0x75
#define PWR_MGMT_1 0x6B
#define ACCEL_XOUT_H 0x3B
#define GYRO_XOUT_H 0x43
#define ACCEL_CONFIG 0x1C
#define GYRO_CONFIG 0x1B

// スケールファクター
#define ACCEL_SCALE_2G 16384.0
#define GYRO_SCALE_250DPS 131.0

bool initMPU6500();
bool readAccelData(float &ax, float &ay, float &az);
bool readGyroData(float &gx, float &gy, float &gz);
uint8_t readRegister(uint8_t reg);
bool writeRegister(uint8_t reg, uint8_t value);

// 補正用オフセット（静止状態でキャリブレーション）
float accelOffsetX = 0, accelOffsetY = 0, accelOffsetZ = 0;
float gyroOffsetX = 0, gyroOffsetY = 0, gyroOffsetZ = 0;

// フィルタ出力（初期化）
float filteredAx = 0, filteredAy = 0, filteredAz = 0;
float filteredGx = 0, filteredGy = 0, filteredGz = 0;

// ローパスフィルタ係数（0 < α < 1） α小 → なめらか
const float alpha = 0.1;

void calibrateSensors() {
  const int samples = 100;
  float sumAx = 0, sumAy = 0, sumAz = 0;
  float sumGx = 0, sumGy = 0, sumGz = 0;
  
  Serial.print("Calibrating... Please keep device still ");
  
  for (int i = 0; i < samples; i++) {
    float ax, ay, az, gx, gy, gz;
    
    readAccelData(ax, ay, az);
    readGyroData(gx, gy, gz);
    
    sumAx += ax;
    sumAy += ay;
    sumAz += az - 1.0; // 重力1g分を除外
    sumGx += gx;
    sumGy += gy;
    sumGz += gz;

    delay(10);
    Serial.print(".");
  }
  Serial.println(" done.");
  
  accelOffsetX = sumAx / samples;
  accelOffsetY = sumAy / samples;
  accelOffsetZ = sumAz / samples;
  gyroOffsetX = sumGx / samples;
  gyroOffsetY = sumGy / samples;
  gyroOffsetZ = sumGz / samples;
}



void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("MPU6500 Setup");
  Serial.println("==============");
  
  // ESP32のI2C初期化
  Wire.begin(21, 22); // SDA=GPIO21, SCL=GPIO22
  Wire.setClock(100000); // 100kHz
  
  delay(500);
  
  if (initMPU6500()) {
   Serial.println("✓ MPU6500 initialization successful!");
   calibrateSensors(); // ← 静止状態でキャリブレーション
  } else {
    Serial.println("✗ MPU6500 initialization failed!");
    while(1) delay(1000);
  }

  Serial.println("Starting data acquisition...");
  Serial.println();
}

void loop() {
  float ax, ay, az;
  float gx, gy, gz;
  
  if (readAccelData(ax, ay, az)) {
    // オフセット補正
    ax -= accelOffsetX;
    ay -= accelOffsetY;
    az -= accelOffsetZ;

    // ローパスフィルタ
    filteredAx = alpha * ax + (1 - alpha) * filteredAx;
    filteredAy = alpha * ay + (1 - alpha) * filteredAy;
    filteredAz = alpha * az + (1 - alpha) * filteredAz;

    Serial.print("Accel - X: ");
    Serial.print(filteredAx, 3);
    Serial.print(" g, Y: ");
    Serial.print(filteredAy, 3);
    Serial.print(" g, Z: ");
    Serial.print(filteredAz, 3);
    Serial.print(" g");
  } else {
    Serial.print("Accel read failed");
  }

if (readGyroData(gx, gy, gz)) {
  // オフセット補正
  gx -= gyroOffsetX;
  gy -= gyroOffsetY;
  gz -= gyroOffsetZ;

  // ローパスフィルタ
  filteredGx = alpha * gx + (1 - alpha) * filteredGx;
  filteredGy = alpha * gy + (1 - alpha) * filteredGy;
  filteredGz = alpha * gz + (1 - alpha) * filteredGz;

  Serial.print(" | Gyro - X: ");
  Serial.print(filteredGx, 1);
  Serial.print(" °/s, Y: ");
  Serial.print(filteredGy, 1);
  Serial.print(" °/s, Z: ");
  Serial.print(filteredGz, 1);
  Serial.print(" °/s");
} else {
  Serial.print(" | Gyro read failed");
}

  
  Serial.println();
  delay(500);
}

bool initMPU6500() {
  // 1. デバイス確認
  uint8_t whoAmI = readRegister(WHO_AM_I_REG);
  Serial.print("WHO_AM_I: 0x");
  Serial.println(whoAmI, HEX);
  
  if (whoAmI != 0x70) {
    Serial.println("Not a MPU6500 device");
    return false;
  }
  
  // 2. デバイスリセット
  if (!writeRegister(PWR_MGMT_1, 0x80)) {
    Serial.println("Reset failed");
    return false;
  }
  delay(100);
  
  // 3. スリープ解除
  if (!writeRegister(PWR_MGMT_1, 0x00)) {
    Serial.println("Wake up failed");
    return false;
  }
  delay(100);
  
  // 4. 加速度計設定 (±2g)
  if (!writeRegister(ACCEL_CONFIG, 0x00)) {
    Serial.println("Accelerometer config failed");
    return false;
  }
  
  // 5. ジャイロ設定 (±250°/s)
  if (!writeRegister(GYRO_CONFIG, 0x00)) {
    Serial.println("Gyroscope config failed");
    return false;
  }
  
  delay(100);
  
  // 6. 設定確認
  uint8_t pwrMgmt = readRegister(PWR_MGMT_1);
  Serial.print("Power Management: 0x");
  Serial.println(pwrMgmt, HEX);
  
  return true;
}

bool readAccelData(float &ax, float &ay, float &az) {
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(ACCEL_XOUT_H);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  
  if (Wire.requestFrom((uint8_t)MPU6500_ADDRESS, (uint8_t)6) != 6) {
    return false;
  }
  
  int16_t rawX = (Wire.read() << 8) | Wire.read();
  int16_t rawY = (Wire.read() << 8) | Wire.read();
  int16_t rawZ = (Wire.read() << 8) | Wire.read();
  
  ax = rawX / ACCEL_SCALE_2G;
  ay = rawY / ACCEL_SCALE_2G;
  az = rawZ / ACCEL_SCALE_2G;
  
  return true;
}

bool readGyroData(float &gx, float &gy, float &gz) {
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(GYRO_XOUT_H);
  if (Wire.endTransmission(false) != 0) {
    return false;
  }
  
  if (Wire.requestFrom((uint8_t)MPU6500_ADDRESS, (uint8_t)6) != 6) {
    return false;
  }
  
  int16_t rawX = (Wire.read() << 8) | Wire.read();
  int16_t rawY = (Wire.read() << 8) | Wire.read();
  int16_t rawZ = (Wire.read() << 8) | Wire.read();
  
  gx = rawX / GYRO_SCALE_250DPS;
  gy = rawY / GYRO_SCALE_250DPS;
  gz = rawZ / GYRO_SCALE_250DPS;
  
  return true;
}

uint8_t readRegister(uint8_t reg) {
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(reg);
  if (Wire.endTransmission(false) != 0) {
    return 0xFF;
  }
  
  if (Wire.requestFrom((uint8_t)MPU6500_ADDRESS, (uint8_t)1) != 1) {
    return 0xFF;
  }
  
  return Wire.read();
}

bool writeRegister(uint8_t reg, uint8_t value) {
  Wire.beginTransmission(MPU6500_ADDRESS);
  Wire.write(reg);
  Wire.write(value);
  return Wire.endTransmission() == 0;
}