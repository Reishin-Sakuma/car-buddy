#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_MPU6050.h>
#include <TFT_eSPI.h>
#include "speed.hpp"

extern TFT_eSPI tft;

static Adafruit_MPU6050 mpu;
static bool sensorReady = false;

// I2Cデバイススキャン関数
void scanI2C() {
  Serial.println("Scanning I2C devices...");
  int deviceCount = 0;
  
  for (byte address = 1; address < 127; address++) {
    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16) Serial.print("0");
      Serial.println(address, HEX);
      deviceCount++;
    }
  }
  
  if (deviceCount == 0) {
    Serial.println("No I2C devices found");
  } else {
    Serial.print("Found ");
    Serial.print(deviceCount);
    Serial.println(" device(s)");
  }
}

// WHO_AM_Iレジスタを直接読み取る関数
uint8_t readWhoAmI(uint8_t address) {
  Wire.beginTransmission(address);
  Wire.write(0x75); // WHO_AM_Iレジスタのアドレス
  if (Wire.endTransmission() != 0) {
    return 0xFF; // エラー
  }
  
  Wire.requestFrom((uint8_t)address, (uint8_t)1);
  if (Wire.available()) {
    return Wire.read();
  }
  return 0xFF;
}

// MPU6500用の手動初期化
bool initMPU6500() {
  Serial.println("Attempting MPU6500 manual initialization...");
  
  // PWR_MGMT_1レジスタ(0x6B)をリセット
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x80); // デバイスリセット
  Wire.endTransmission();
  delay(100);
  
  // PWR_MGMT_1レジスタでスリープ解除
  Wire.beginTransmission(0x68);
  Wire.write(0x6B);
  Wire.write(0x00); // スリープ解除、内部クロック使用
  Wire.endTransmission();
  delay(100);
  
  // 加速度センサー設定 (ACCEL_CONFIG: 0x1C)
  Wire.beginTransmission(0x68);
  Wire.write(0x1C);
  Wire.write(0x10); // ±8g range
  Wire.endTransmission();
  
  // ジャイロ設定 (GYRO_CONFIG: 0x1B)
  Wire.beginTransmission(0x68);
  Wire.write(0x1B);
  Wire.write(0x08); // ±500度/秒 range
  Wire.endTransmission();
  
  // CONFIG レジスタ (0x1A) - ローパスフィルタ設定
  Wire.beginTransmission(0x68);
  Wire.write(0x1A);
  Wire.write(0x03); // DLPF_CFG = 3 (44Hz)
  Wire.endTransmission();
  
  delay(100);
  
  // WHO_AM_Iレジスタで確認
  uint8_t whoAmI = readWhoAmI(0x68);
  Serial.print("After manual init, WHO_AM_I: 0x");
  Serial.println(whoAmI, HEX);
  
  return (whoAmI == 0x70);
}

// 手動でセンサーデータを読み取る
bool readMPU6500Data(float &ax, float &ay, float &az) {
  Wire.beginTransmission(0x68);
  Wire.write(0x3B); // ACCEL_XOUT_H レジスタから開始
  if (Wire.endTransmission() != 0) {
    return false;
  }
  
  Wire.requestFrom((uint8_t)0x68, (uint8_t)6); // 6バイト読み取り（X,Y,Z各2バイト）
  
  if (Wire.available() >= 6) {
    int16_t rawAX = (Wire.read() << 8) | Wire.read();
    int16_t rawAY = (Wire.read() << 8) | Wire.read();
    int16_t rawAZ = (Wire.read() << 8) | Wire.read();
    
    // ±8g設定での変換 (LSB感度: 4096 LSB/g)
    ax = rawAX / 4096.0;
    ay = rawAY / 4096.0;
    az = rawAZ / 4096.0;
    
    return true;
  }
  
  return false;
}

bool initSpeedSensor() {
  Wire.begin(21, 22); // SDA, SCLピンを指定（ESP32の例）
  Serial.println("Trying to initialize accelerometer...");
  
  // I2Cデバイススキャンを実行
  scanI2C();
  
  // WHO_AM_Iチェック
  uint8_t whoAmI = readWhoAmI(0x68);
  Serial.print("WHO_AM_I register: 0x");
  Serial.println(whoAmI, HEX);
  
  if (whoAmI == 0x70) {
    Serial.println("Device is MPU6500! Using manual initialization...");
    
    if (initMPU6500()) {
      sensorReady = true;
      Serial.println("MPU6500 manual initialization successful!");
      return true;
    }
  } else {
    Serial.println("Attempting standard MPU6050 initialization...");
    
    if (mpu.begin(0x68, &Wire)) {
      sensorReady = true;
      Serial.println("Standard MPU6050 initialization successful!");
      
      // センサー設定
      mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
      mpu.setGyroRange(MPU6050_RANGE_500_DEG);
      mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
      
      return true;
    }
  }
  
  Serial.println("All initialization attempts failed!");
  sensorReady = false;
  return false;
}

float getSpeed() {
  if (!sensorReady) return 0.0;
  
  // WHO_AM_Iに基づいて適切な読み取り方法を選択
  uint8_t whoAmI = readWhoAmI(0x68);
  
  if (whoAmI == 0x70) {
    // MPU6500の場合、手動でデータ読み取り
    float ax, ay, az;
    if (readMPU6500Data(ax, ay, az)) {
      return ax; // X軸加速度を返す
    } else {
      Serial.println("Failed to read MPU6500 data");
      return 0.0;
    }
  } else {
    // 標準のAdafruitライブラリを使用
    sensors_event_t a, g, temp;
    if (mpu.getEvent(&a, &g, &temp)) {
      return a.acceleration.x;
    } else {
      Serial.println("Failed to read sensor data");
      return 0.0;
    }
  }
}