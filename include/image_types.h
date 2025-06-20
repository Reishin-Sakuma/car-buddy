// image_types.h - 画像関連の共通構造体定義
#ifndef IMAGE_TYPES_H
#define IMAGE_TYPES_H

#include <Arduino.h>

// 画像データ構造体（キャラクター画像用）
typedef struct {
    const uint16_t *data;
    uint16_t width;
    uint16_t height;
    uint8_t dataSize;
} tImage;

#endif // IMAGE_TYPES_H