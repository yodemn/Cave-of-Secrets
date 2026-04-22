#ifndef LEVEL_H
#define LEVEL_H

#include <stdint.h>
extern const uint8_t level_1_map[8][10];
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

struct LevelPlatformColumn {
  int16_t x;
  int16_t y;
  uint8_t tiles;
};

enum LevelObjectType {
  LEVEL_OBJECT_SMALL_TREE,
  LEVEL_OBJECT_LARGE_TREE,
  LEVEL_OBJECT_CHEST,
  LEVEL_OBJECT_SPIKE_UP,
  LEVEL_OBJECT_SPIKE_DOWN,
  LEVEL_OBJECT_SPIKE_LEFT,
  LEVEL_OBJECT_SPIKE_RIGHT
};

struct LevelObject {
  LevelObjectType type;
  int16_t x;
  int16_t y;
};

struct LevelDefinition {
  const LevelPlatform *platforms;
  uint8_t platformCount;
  const LevelPlatformColumn *columns;
  uint8_t columnCount;
  const LevelObject *objects;
  uint8_t objectCount;
};

extern const uint8_t LevelCount;
extern const LevelDefinition Levels[];
bool IsPlayerTouchingSpike(uint8_t levelIndex, Rect playerArea);
void DrawLevel(uint8_t levelIndex);
void RedrawLevelPiecesInArea(uint8_t levelIndex, Rect outdatedArea);
void ResetLevelObjectStates(uint8_t levelIndex);
bool TryOpenNearbyChest(uint8_t levelIndex, Rect playerArea, Rect *dirtyArea);
bool UpdateLevelAnimations(uint8_t levelIndex, Rect *dirtyArea);
bool FindPlatformLanding(uint8_t levelIndex, Rect previousPlayerArea, Rect currentPlayerArea, int16_t *landingY);
bool FindPlatformColumnSideCollision(uint8_t levelIndex, Rect previousPlayerArea, Rect currentPlayerArea, int16_t *blockedX);
bool IsPlayerSupportedByPlatform(uint8_t levelIndex, Rect playerArea);
void mainMenu(const char* title, const char* options[], int numOptions, int language);
void DrawCursor(int oldSelection, int newSelection);
#endif
