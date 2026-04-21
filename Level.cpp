#include "Level.h"
#include "TileSprites.h"
#include "../inc/ST7735.h"

static const LevelPlatform Level0Platforms[] = {
  {6, 118, 7},
  {54, 96, 6},
  {13, 74, 5},
  {96, 68, 5},
  {38, 48, 7},
  {79, 29, 5}
};

static const LevelObject Level0Objects[] = {
  {LEVEL_OBJECT_SMALL_TREE, 2, 127},
  {LEVEL_OBJECT_LARGE_TREE, 103, 127}
};

const LevelDefinition Levels[] = {
  {
    Level0Platforms,
    sizeof(Level0Platforms) / sizeof(Level0Platforms[0]),
    Level0Objects,
    sizeof(Level0Objects) / sizeof(Level0Objects[0])
  }
};

const uint8_t LevelCount = sizeof(Levels) / sizeof(Levels[0]);

static bool AreasOverlap(Rect a, Rect b){
  return (a.x0 <= b.x1) && (a.x1 >= b.x0) && (a.y0 <= b.y1) && (a.y1 >= b.y0);
}

static Rect GetImageArea(int16_t x, int16_t y, ImageData image){
  Rect r;
  r.x0 = x;
  r.y0 = y - image.height + 1;
  r.x1 = x + image.width - 1;
  r.y1 = y;
  return r;
}

static Rect GetPlatformArea(const LevelPlatform &platform){
  Rect r;
  r.x0 = platform.x;
  r.y0 = platform.y - TILE_SPRITE_HEIGHT + 1;
  r.x1 = platform.x + platform.tiles * TILE_SPRITE_WIDTH - 1;
  r.y1 = platform.y;
  return r;
}

static int16_t GetPlatformLandingY(const LevelPlatform &platform){
  return platform.y - TILE_SPRITE_HEIGHT;
}

static bool PlayerFeetOverlapPlatform(Rect playerArea, const LevelPlatform &platform){
  int16_t platformX0 = platform.x;
  int16_t platformX1 = platform.x + platform.tiles * TILE_SPRITE_WIDTH - 1;
  int16_t playerFootX0 = playerArea.x0 + 2;
  int16_t playerFootX1 = playerArea.x1 - 2;
  return (playerFootX0 <= platformX1) && (playerFootX1 >= platformX0);
}

static ImageData ObjectImage(LevelObjectType type){
  if(type == LEVEL_OBJECT_LARGE_TREE){
    return LargeTreeImage;
  }
  return SmallTreeImage;
}

static void DrawObject(const LevelObject &object){
  DrawImageChroma(object.x, object.y, ObjectImage(object.type));
}

static void DrawObjectInsideArea(const LevelObject &object, Rect redrawArea){
  ImageData image = ObjectImage(object.type);
  Rect imageArea = GetImageArea(object.x, object.y, image);
  if(!AreasOverlap(imageArea, redrawArea)){
    return;
  }

  int16_t x0 = (imageArea.x0 > redrawArea.x0) ? imageArea.x0 : redrawArea.x0;
  int16_t y0 = (imageArea.y0 > redrawArea.y0) ? imageArea.y0 : redrawArea.y0;
  int16_t x1 = (imageArea.x1 < redrawArea.x1) ? imageArea.x1 : redrawArea.x1;
  int16_t y1 = (imageArea.y1 < redrawArea.y1) ? imageArea.y1 : redrawArea.y1;

  for(int16_t screenY = y0; screenY <= y1; screenY++){
    int16_t row = screenY - imageArea.y0;
    int16_t sourceRow = image.height - 1 - row;
    for(int16_t screenX = x0; screenX <= x1; screenX++){
      int16_t sourceCol = screenX - imageArea.x0;
      uint16_t color = image.pixels[sourceRow * image.width + sourceCol];
      if(color != TILE_CHROMA_KEY){
        ST7735_DrawPixel(screenX, screenY, color);
      }
    }
  }
}

bool FindPlatformLanding(uint8_t levelIndex, Rect previousPlayerArea, Rect currentPlayerArea, int16_t *landingY){
  if(levelIndex >= LevelCount){
    return false;
  }

  const LevelDefinition &level = Levels[levelIndex];
  bool foundLanding = false;
  int16_t bestLandingY = 127;

  for(uint8_t i = 0; i < level.platformCount; i++){
    const LevelPlatform &platform = level.platforms[i];
    int16_t platformLandingY = GetPlatformLandingY(platform);
    bool crossedPlatformTop = (previousPlayerArea.y1 <= platformLandingY) && (currentPlayerArea.y1 >= platformLandingY);

    if(crossedPlatformTop && PlayerFeetOverlapPlatform(currentPlayerArea, platform)){
      if(!foundLanding || platformLandingY < bestLandingY){
        bestLandingY = platformLandingY;
        foundLanding = true;
      }
    }
  }

  if(foundLanding && landingY){
    *landingY = bestLandingY;
  }
  return foundLanding;
}

bool IsPlayerSupportedByPlatform(uint8_t levelIndex, Rect playerArea){
  if(levelIndex >= LevelCount){
    return false;
  }

  const LevelDefinition &level = Levels[levelIndex];
  for(uint8_t i = 0; i < level.platformCount; i++){
    const LevelPlatform &platform = level.platforms[i];
    if(playerArea.y1 == GetPlatformLandingY(platform) && PlayerFeetOverlapPlatform(playerArea, platform)){
      return true;
    }
  }
  return false;
}

void DrawLevel(uint8_t levelIndex){
  if(levelIndex >= LevelCount){
    return;
  }

  const LevelDefinition &level = Levels[levelIndex];
  for(uint8_t i = 0; i < level.objectCount; i++){
    DrawObject(level.objects[i]);
  }
  for(uint8_t i = 0; i < level.platformCount; i++){
    DrawPlatformRun(level.platforms[i].x, level.platforms[i].y, level.platforms[i].tiles);
  }
}

void RedrawLevelPiecesInArea(uint8_t levelIndex, Rect outdatedArea){
  if(levelIndex >= LevelCount){
    return;
  }

  const LevelDefinition &level = Levels[levelIndex];
  for(uint8_t i = 0; i < level.objectCount; i++){
    if(AreasOverlap(outdatedArea, GetImageArea(level.objects[i].x, level.objects[i].y, ObjectImage(level.objects[i].type)))){
      DrawObjectInsideArea(level.objects[i], outdatedArea);
    }
  }
  for(uint8_t i = 0; i < level.platformCount; i++){
    if(AreasOverlap(outdatedArea, GetPlatformArea(level.platforms[i]))){
      DrawPlatformRun(level.platforms[i].x, level.platforms[i].y, level.platforms[i].tiles);
    }
  }
}
