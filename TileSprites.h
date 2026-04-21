#ifndef TILESPRITES_H
#define TILESPRITES_H

#include <stdint.h>
#include "img.h"

#define TILE_SPRITE_WIDTH 9
#define TILE_SPRITE_HEIGHT 11
#define TILE_SOURCE_WIDTH 16
#define TILE_SOURCE_HEIGHT 16
#define TILE_CHROMA_KEY 0x07E0
#define LARGE_TREE_WIDTH 56
#define LARGE_TREE_HEIGHT 82
#define SMALL_TREE_WIDTH 36
#define SMALL_TREE_HEIGHT 56
#define CHEST_SPRITE_WIDTH 24
#define CHEST_SPRITE_HEIGHT 14
#define CHEST_OPEN_FRAME_COUNT 4

extern const uint16_t LargeTreeSprite[LARGE_TREE_WIDTH * LARGE_TREE_HEIGHT];
extern ImageData LargeTreeImage;
extern const uint16_t SmallTreeSprite[SMALL_TREE_WIDTH * SMALL_TREE_HEIGHT];
extern ImageData SmallTreeImage;
extern ImageData ChestOpenFrames[CHEST_OPEN_FRAME_COUNT];

extern const uint16_t TileSprite_05_04[TILE_SPRITE_WIDTH * TILE_SPRITE_HEIGHT];
extern ImageData TileImage_05_04;
extern const uint16_t TileSprite_06_04[TILE_SPRITE_WIDTH * TILE_SPRITE_HEIGHT];
extern ImageData TileImage_06_04;
extern const uint16_t TileSprite_07_04[TILE_SPRITE_WIDTH * TILE_SPRITE_HEIGHT];
extern ImageData TileImage_07_04;
extern const uint16_t TileSprite_08_04[TILE_SPRITE_WIDTH * TILE_SPRITE_HEIGHT];
extern ImageData TileImage_08_04;
extern const uint16_t TileSprite_09_04[TILE_SPRITE_WIDTH * TILE_SPRITE_HEIGHT];
extern ImageData TileImage_09_04;

// Platform surface pieces from tileset row 4, columns 5-9.
#define PlatformLeftEndImage TileImage_05_04
#define PlatformMiddleAImage TileImage_06_04
#define PlatformMiddleBImage TileImage_07_04
#define PlatformMiddleCImage TileImage_08_04
#define PlatformRightEndImage TileImage_09_04

void DrawBitmapChroma(int16_t x, int16_t y, const uint16_t *image, int16_t w, int16_t h, uint16_t chroma);
void DrawImageChroma(int16_t x, int16_t y, ImageData image, uint16_t chroma = TILE_CHROMA_KEY);
void DrawPlatformRun(int16_t x, int16_t y, uint8_t tiles);

#endif
