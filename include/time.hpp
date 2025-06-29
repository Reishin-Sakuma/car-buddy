#pragma once

#include <Arduino.h>

// 時刻システム初期化
void initTimeSystem();

// 時刻の保存・復元
void saveCurrentTime();
void restoreTimeFromEEPROM();

// 時刻取得関数（タイムアウト設定済み）
String getCurrentTimeString();
String getCurrentTime();
String getCurrentDate();

// 時刻有効性チェック
bool isTimeValid();