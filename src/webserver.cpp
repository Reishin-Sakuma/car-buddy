#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "webserver.hpp"
#include "time.hpp"
#include <time.h>

// 内部インスタンス
static WebServer server(80);
static bool serverRunning = false;

// HTTPハンドラー関数の前方宣言
void handleRoot();
void handleSetTime();
bool parseAndSetTime(String timeStr);

int getConnectedClientCount() {
    return WiFi.softAPgetStationNum();
}

bool isClientConnected() {
    return (getConnectedClientCount() > 0);
}

void initWebServer() {
    Serial.println("Initializing web server...");
    
    // WiFi完全初期化
    WiFi.mode(WIFI_OFF);
    delay(100);
    WiFi.mode(WIFI_AP_STA);
    delay(200);
    
    // TCP/IPスタック強制初期化
    WiFi.softAP("CarBuddy-WiFi", "carbuddy123");
    delay(500);  // AP安定化待機
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    // Webサーバー設定
    server.on("/", handleRoot);
    server.on("/settime", HTTP_POST, handleSetTime);
    server.enableCORS(true);
    server.begin();
    
    serverRunning = true;
    Serial.println("Web server started successfully");
}

void handleWebServerClient() {
    if (serverRunning) {
        server.handleClient();
    }
}

void stopWebServer() {
    if (serverRunning) {
        server.stop();
        WiFi.softAPdisconnect(true);
        serverRunning = false;
        Serial.println("Web server stopped");
    }
}

// HTTPハンドラー実装 - 簡素化版
void handleRoot() {
    String html = "<!DOCTYPE html>";
    html += "<html><head><title>CarBuddy Time</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "</head><body>";
    html += "<h1>CarBuddy Time Sync</h1>";
    html += "<p>Current Time: <span id='time'></span></p>";
    html += "<button onclick='sync()'>Sync Time</button>";
    html += "<p id='result'></p>";
    html += "<script>";
    
    // 時刻表示用のシンプルなJavaScript
    html += "function updateTime() {";
    html += "const now = new Date();";
    html += "document.getElementById('time').textContent = now.toLocaleString();";
    html += "}";
    
    // 時刻同期用のシンプルなJavaScript
    html += "function sync() {";
    html += "const now = new Date();";
    html += "const timeStr = now.getFullYear() + '-' + ";
    html += "String(now.getMonth() + 1).padStart(2, '0') + '-' + ";
    html += "String(now.getDate()).padStart(2, '0') + ' ' + ";
    html += "String(now.getHours()).padStart(2, '0') + ':' + ";
    html += "String(now.getMinutes()).padStart(2, '0') + ':' + ";
    html += "String(now.getSeconds()).padStart(2, '0');";
    html += "fetch('/settime', {";
    html += "method: 'POST',";
    html += "headers: {'Content-Type': 'application/x-www-form-urlencoded'},";
    html += "body: 'time=' + timeStr";
    html += "})";
    html += ".then(response => response.text())";
    html += ".then(data => document.getElementById('result').textContent = data)";
    html += ".catch(error => document.getElementById('result').textContent = 'Error: ' + error);";
    html += "}";
    
    html += "setInterval(updateTime, 1000);";
    html += "updateTime();";
    html += "</script></body></html>";
    
    server.send(200, "text/html", html);
}

void handleSetTime() {
    if (server.hasArg("time")) {
        String timeStr = server.arg("time");
        Serial.print("Time sync: ");
        Serial.println(timeStr);
        
        if (parseAndSetTime(timeStr)) {
            // 時刻をEEPROMに保存
            saveCurrentTime();
            server.send(200, "text/plain", "OK");
            Serial.println("Time sync OK");
        } else {
            server.send(400, "text/plain", "Format Error");
            Serial.println("Time format error");
        }
    } else {
        server.send(400, "text/plain", "No Data");
        Serial.println("No time data");
    }
}

bool parseAndSetTime(String timeStr) {
    struct tm timeinfo;
    int year, month, day, hour, minute, second;
    
    int parsed = sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d", 
                       &year, &month, &day, &hour, &minute, &second);
    
    if (parsed != 6) {
        return false;
    }
    
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = day;
    timeinfo.tm_hour = hour;
    timeinfo.tm_min = minute;
    timeinfo.tm_sec = second;
    
    time_t timestamp = mktime(&timeinfo);
    struct timeval tv = { timestamp, 0 };
    settimeofday(&tv, NULL);
    
    return true;
}