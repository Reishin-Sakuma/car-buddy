#include <Arduino.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <WebServer.h>
#include <time.h>
#include <EEPROM.h>

TFT_eSPI tft = TFT_eSPI();
WebServer server(80);

// 時刻表示用
static String lastTime = "";
static String lastDate = "";

// 関数宣言
void updateTimeDisplay();
bool parseAndSetTime(String timeStr);
String getCurrentTimeString();
void handleSetTime();
void handleRoot();

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("=== CarBuddy WiFi AP Time Sync ===");

    // TFT初期化
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLUE);
    
    // タイトル表示
    tft.setTextColor(TFT_WHITE, TFT_BLUE);
    tft.setTextSize(3);
    tft.drawString("CarBuddy", 50, 10);
    
    // ラベル表示
    tft.setTextSize(2);
    tft.drawString("Time:", 20, 60);
    tft.drawString("Date:", 20, 90);
    tft.drawString("WiFi:", 20, 120);
    tft.drawString("IP:", 20, 140);
    tft.drawString("Status:", 20, 170);
    
    // EEPROM初期化
    EEPROM.begin(512);
    
    // WiFiアクセスポイント設定
    Serial.println("Setting up WiFi Access Point...");
    WiFi.softAP("CarBuddy-WiFi", "carbuddy123");
    
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);
    
    // 画面にIP表示
    tft.setTextColor(TFT_GREEN, TFT_BLUE);
    tft.setTextSize(2);
    tft.drawString("Active", 100, 120);
    tft.drawString(IP.toString(), 100, 140);
    
    // Webサーバー設定
    server.on("/", handleRoot);
    server.on("/settime", HTTP_POST, handleSetTime);
    server.enableCORS(true);
    server.begin();
    
    Serial.println("Web server started");
    Serial.println("WiFi SSID: CarBuddy-WiFi");
    Serial.println("Password: carbuddy123");
    Serial.println("URL: http://" + IP.toString() + "/settime");
    
    // 保存された時刻復元
    time_t savedTime;
    EEPROM.get(0, savedTime);
    if (savedTime > 1700000000) {
        struct timeval tv = { savedTime, 0 };
        settimeofday(&tv, NULL);
        Serial.println("Time restored from EEPROM");
        updateTimeDisplay();
    } else {
        tft.setTextColor(TFT_YELLOW, TFT_BLUE);
        tft.drawString("--:--", 100, 60);
        tft.drawString("----/--/--", 100, 90);
    }
    
    Serial.println("=== Ready for WiFi time sync ===");
}

void loop() {
    server.handleClient();
    
    // 時刻更新
    static unsigned long lastUpdate = 0;
    if (millis() - lastUpdate > 1000) {
        updateTimeDisplay();
        lastUpdate = millis();
    }
    
    // 接続クライアント数表示
    static unsigned long lastClientCheck = 0;
    if (millis() - lastClientCheck > 5000) {
        int clients = WiFi.softAPgetStationNum();
        Serial.print("Connected clients: ");
        Serial.println(clients);
        
        // 画面更新
        tft.fillRect(100, 170, 200, 20, TFT_BLUE);
        tft.setTextColor(TFT_WHITE, TFT_BLUE);
        tft.setTextSize(2);
        if (clients > 0) {
            tft.drawString("Connected", 100, 170);
        } else {
            tft.drawString("Waiting", 100, 170);
        }
        
        lastClientCheck = millis();
    }
    
    delay(10);
}

void handleRoot() {
    String html = "<!DOCTYPE html>";
    html += "<html><head><title>CarBuddy Time Sync</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "body { font-family: Arial; margin: 40px; text-align: center; }";
    html += "button { padding: 15px 30px; font-size: 18px; margin: 10px; }";
    html += ".time { font-size: 24px; margin: 20px; }";
    html += "</style></head><body>";
    html += "<h1>CarBuddy Time Sync</h1>";
    html += "<div class='time' id='currentTime'></div>";
    html += "<button onclick='syncTime()'>Sync Time</button>";
    html += "<div id='status'></div>";
    html += "<script>";
    html += "function updateTime() {";
    html += "const now = new Date();";
    html += "document.getElementById('currentTime').innerHTML = ";
    html += "now.getFullYear() + '-' + ";
    html += "String(now.getMonth() + 1).padStart(2, '0') + '-' + ";
    html += "String(now.getDate()).padStart(2, '0') + ' ' + ";
    html += "String(now.getHours()).padStart(2, '0') + ':' + ";
    html += "String(now.getMinutes()).padStart(2, '0') + ':' + ";
    html += "String(now.getSeconds()).padStart(2, '0');";
    html += "}";
    html += "function syncTime() {";
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
    html += ".then(data => {";
    html += "document.getElementById('status').innerHTML = ";
    html += "'<p style=\"color: green;\">OK: ' + data + '</p>';";
    html += "})";
    html += ".catch(error => {";
    html += "document.getElementById('status').innerHTML = ";
    html += "'<p style=\"color: red;\">Error: ' + error + '</p>';";
    html += "});";
    html += "}";
    html += "setInterval(updateTime, 1000);";
    html += "updateTime();";
    html += "</script></body></html>";
    
    server.send(200, "text/html", html);
}

void handleSetTime() {
    if (server.hasArg("time")) {
        String timeStr = server.arg("time");
        Serial.print("Received time: ");
        Serial.println(timeStr);
        
        if (parseAndSetTime(timeStr)) {
            // EEPROM保存
            time_t now;
            time(&now);
            EEPROM.put(0, now);
            EEPROM.commit();
            
            // 画面更新
            tft.fillRect(100, 170, 200, 20, TFT_BLUE);
            tft.setTextColor(TFT_GREEN, TFT_BLUE);
            tft.setTextSize(2);
            tft.drawString("Time Set!", 100, 170);
            
            server.send(200, "text/plain", "Time sync complete: " + timeStr);
            Serial.println("Time sync successful");
        } else {
            server.send(400, "text/plain", "Time format error");
            Serial.println("Time format error");
        }
    } else {
        server.send(400, "text/plain", "No time data");
    }
}

bool parseAndSetTime(String timeStr) {
    struct tm timeinfo;
    int year, month, day, hour, minute, second;
    
    int parsed = sscanf(timeStr.c_str(), "%d-%d-%d %d:%d:%d", 
                       &year, &month, &day, &hour, &minute, &second);
    
    if (parsed != 6) return false;
    
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

void updateTimeDisplay() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return;
    
    char timeStr[10];
    strftime(timeStr, sizeof(timeStr), "%H:%M:%S", &timeinfo);
    String currentTime = String(timeStr);
    
    char dateStr[12];
    strftime(dateStr, sizeof(dateStr), "%Y/%m/%d", &timeinfo);
    String currentDate = String(dateStr);
    
    if (currentTime != lastTime) {
        tft.fillRect(100, 60, 150, 20, TFT_BLUE);
        tft.setTextColor(TFT_YELLOW, TFT_BLUE);
        tft.setTextSize(2);
        tft.drawString(currentTime, 100, 60);
        lastTime = currentTime;
    }
    
    if (currentDate != lastDate) {
        tft.fillRect(100, 90, 150, 20, TFT_BLUE);
        tft.setTextColor(TFT_CYAN, TFT_BLUE);
        tft.setTextSize(2);
        tft.drawString(currentDate, 100, 90);
        lastDate = currentDate;
    }
}

String getCurrentTimeString() {
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) return "1970-01-01 00:00:00";
    
    char buffer[20];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return String(buffer);
}