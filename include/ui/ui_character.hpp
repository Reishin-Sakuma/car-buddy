#ifndef UI_CHARACTER_HPP
#define UI_CHARACTER_HPP

#include <Arduino.h>

// キャラクター表示機能
const uint16_t* getCharacterImageArray(float temp);
void drawCharacterImageWithFade(int x, int y);
void drawCharacterImage(int x, int y);
void drawCharacterImageWithEdgeFade(int x, int y);
void drawCharacter();
void clearCharacterArea();
void debugCharacterState();  // 追加：デバッグ用

#endif