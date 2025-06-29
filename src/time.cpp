#include "time.hpp"
#include <time.h>
#include <EEPROM.h>

#define TIME_EEPROM_ADDRESS 100  // 温度センサー用と重複しないアドレス

void initTimeSystem() {
    Serial.println("Initializing time system...");
    
    // EEPROMから時刻復元を試行
    restoreTimeFromEEPROM();
    
    Serial.println("Time system ready");
}

void saveCurrentTime() {
    time_t now;
    time(&now);
    
    // EEPROMに現在時刻を保存
    EEPROM.put(TIME_EEPROM_ADDRESS, now);
    EEPROM.commit();
    
    Serial.print("Time saved to EEPROM: ");
    Serial.println(now);
}

void restoreTimeFromEEPROM() {
    time_t savedTime;
    EEPROM.get(TIME_EEPROM_ADDRESS, savedTime);
    
    // 有効な時刻かチェック（2024年以降）
    if (savedTime > 1700000000) {
        struct timeval tv = { savedTime, 0 };
        settimeofday(&tv, NULL);
        
        Serial.print("Time restored from EEPROM: ");
        Serial.println(getCurrentTimeString());
    } else {
        Serial.println("No valid time found in EEPROM");
    }
}

String getCurrentTimeString() {
    struct tm timeinfo;
    // 重要: 短いタイムアウト（100ms）を設定してブロッキングを防止
    if (!getLocalTime(&timeinfo, 100)) {
        return "1970-01-01 00:00:00";
    }
    
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}

String getCurrentTime() {
    struct tm timeinfo;
    // 重要: 短いタイムアウト（50ms）を設定してメインループの遅延を防止
    if (!getLocalTime(&timeinfo, 50)) {
        return "--:--";
    }
    
    char timeStr[6];
    strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
    return String(timeStr);
}

String getCurrentDate() {
    struct tm timeinfo;
    // 重要: 短いタイムアウト（50ms）を設定してメインループの遅延を防止
    if (!getLocalTime(&timeinfo, 50)) {
        return "----/--/--";
    }
    
    char dateStr[11];
    strftime(dateStr, sizeof(dateStr), "%Y/%m/%d", &timeinfo);
    return String(dateStr);
}

bool isTimeValid() {
    struct tm timeinfo;
    // 短いタイムアウトで同期状態をチェック
    return getLocalTime(&timeinfo, 100);
}