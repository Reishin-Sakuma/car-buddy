#include <Arduino.h>
#include <TFT_eSPI.h>
#include "ui.hpp"
#include "../include/temperature.hpp"
#include "../include/speed.hpp"
#include "../include/time.hpp"
#include "../include/ui/ui_display.hpp"
#include "../include/ui/ui_state.hpp"
#include "../include/ui/ui_temperature.hpp"
#include "../include/ui/ui_character.hpp"
#include "../include/ui/ui_data.hpp"
#include "../include/ui/ui_compatibility.hpp"

extern TFT_eSPI tft;

// 状態管理変数
bool uiInitialized = false;
float lastTemperature = -999.0;
float lastSpeed = -999.0;
String lastTime = "";
String lastDate = "";
bool characterDisplayed = false;
bool isHotCharacterMode = false;

// 温度連動背景色用の変数
float currentBackgroundTemp = 20.0;  // デフォルト温度
float lastBackgroundUpdateTemp = -999.0;