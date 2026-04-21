#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>

struct Rect {
  int16_t x0;
  int16_t y0;
  int16_t x1;
  int16_t y1;
};

struct LevelPlatform {
  int16_t x;
  int16_t y;
  uint8_t tiles;
};

enum LevelObjectType {
  LEVEL_OBJECT_SMALL_TREE,
  LEVEL_OBJECT_LARGE_TREE
};

struct LevelObject {
  LevelObjectType type;
  int16_t x;
  int16_t y;
};

struct LevelDefinition {
  const LevelPlatform *platforms;
  uint8_t platformCount;
  const LevelObject *objects;
  uint8_t objectCount;
};

extern const uint8_t LevelCount;
extern const LevelDefinition Levels[];

void DrawLevel(uint8_t levelIndex);
void RedrawLevelPiecesInArea(uint8_t levelIndex, Rect outdatedArea);

#endif
