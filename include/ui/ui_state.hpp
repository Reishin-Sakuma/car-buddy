#ifndef UI_STATE_HPP
#define UI_STATE_HPP

#include <Arduino.h>

// 状態管理機能
void forceUpdateAllDisplayValues();
void setLastDisplayValues(float temp, float speed, String timeStr, String dateStr);
void forceFullRedraw(float temp);

// 状態取得機能
bool isUIInitialized();
void setUIInitialized(bool initialized);

// 前回値取得機能（他のモジュールから参照用）
float getLastTemperature();
float getLastSpeed();
String getLastTime();
String getLastDate();
bool isCharacterDisplayed();

#endif