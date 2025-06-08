#ifndef SPEED_H
#define SPEED_H

bool initSpeedSensor();
float readSpeed();  // 加速度センサーから速度を読み取る関数
float getSpeed();  // 今回は加速度センサーのX軸を仮の「速度」として返す

// void drawSpeed(float speed);  // UIに速度（加速度値）を表示

#endif
