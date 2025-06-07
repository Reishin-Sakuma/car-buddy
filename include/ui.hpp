#ifndef UI_HPP
#define UI_HPP

#include <TFT_eSPI.h>
extern TFT_eSPI tft;  // ← これがないとtftが未定義になる

void drawUI(float speed);
void drawCharacterImage(int x, int y);
void drawTemperature(float temp);


#endif
