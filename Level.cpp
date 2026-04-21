#include "Level.h"
#include "TileSprites.h"
#include "../inc/ST7735.h"

static const LevelPlatform Level0Platforms[] = {
  {8, 116, 6},
  {54, 96, 7},
  {12, 78, 5},
  {94, 70, 5},
  {36, 52, 8},
  {73, 35, 6}
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

static bool RectsIntersect(LevelRect a, LevelRect b){
  return (a.x0 <= b.x1) && (a.x1 >= b.x0) && (a.y0 <= b.y1) && (a.y1 >= b.y0);
}

static LevelRect ImageRect(int16_t x, int16_t y, ImageData image){
  LevelRect r;
  r.x0 = x;
  r.y0 = y - image.height + 1;
  r.x1 = x + image.width - 1;
  r.y1 = y;
  return r;
}

static LevelRect PlatformRect(const LevelPlatform &platform){
  LevelRect r;
  r.x0 = platform.x;
  r.y0 = platform.y - TILE_SPRITE_HEIGHT + 1;
  r.x1 = platform.x + platform.tiles * TILE_SPRITE_WIDTH - 1;
  r.y1 = platform.y;
  return r;
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

static void DrawObjectClipped(const LevelObject &object, LevelRect clip){
  ImageData image = ObjectImage(object.type);
  LevelRect imageRect = ImageRect(object.x, object.y, image);
  if(!RectsIntersect(imageRect, clip)){
    return;
  }

  int16_t x0 = (imageRect.x0 > clip.x0) ? imageRect.x0 : clip.x0;
  int16_t y0 = (imageRect.y0 > clip.y0) ? imageRect.y0 : clip.y0;
  int16_t x1 = (imageRect.x1 < clip.x1) ? imageRect.x1 : clip.x1;
  int16_t y1 = (imageRect.y1 < clip.y1) ? imageRect.y1 : clip.y1;

  for(int16_t screenY = y0; screenY <= y1; screenY++){
    int16_t row = screenY - imageRect.y0;
    int16_t sourceRow = image.height - 1 - row;
    for(int16_t screenX = x0; screenX <= x1; screenX++){
      int16_t sourceCol = screenX - imageRect.x0;
      uint16_t color = image.pixels[sourceRow * image.width + sourceCol];
      if(color != TILE_CHROMA_KEY){
        ST7735_DrawPixel(screenX, screenY, color);
      }
    }
  }
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

void DrawLevelPiecesInRect(uint8_t levelIndex, LevelRect dirty){
  if(levelIndex >= LevelCount){
    return;
  }

  const LevelDefinition &level = Levels[levelIndex];
  for(uint8_t i = 0; i < level.objectCount; i++){
    if(RectsIntersect(dirty, ImageRect(level.objects[i].x, level.objects[i].y, ObjectImage(level.objects[i].type)))){
      DrawObjectClipped(level.objects[i], dirty);
    }
  }
  for(uint8_t i = 0; i < level.platformCount; i++){
    if(RectsIntersect(dirty, PlatformRect(level.platforms[i]))){
      DrawPlatformRun(level.platforms[i].x, level.platforms[i].y, level.platforms[i].tiles);
    }
  }
}
