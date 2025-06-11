#ifndef TIME_H
#define TIME_H

#include <Arduino.h>

// 時刻管理モジュール
void initTimeSystem();
void saveCurrentTime();
void restoreTimeFromEEPROM();
String getCurrentTimeString();
String getCurrentTime();    // HH:MM形式
String getCurrentDate();    // MM/DD形式
bool isTimeValid();

#endif