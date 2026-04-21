#include "Level.h"
#include "TileSprites.h"
#include "../inc/ST7735.h"

static const uint8_t CHEST_OPEN_FRAME_DELAY = 3;
static const int16_t CHEST_INTERACTION_PADDING_X = 8;
static const int16_t CHEST_INTERACTION_PADDING_Y = 8;

struct LevelObjectState {
  uint8_t chestFrame;
  uint8_t chestTick;
  bool chestOpened;
  bool chestAnimating;
};

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
  {LEVEL_OBJECT_LARGE_TREE, 103, 127},
  {LEVEL_OBJECT_CHEST, 50, 107},
  {LEVEL_OBJECT_CHEST, 80, 85},
  {LEVEL_OBJECT_CHEST, 30, 63},
  {LEVEL_OBJECT_CHEST, 108, 57}
};

static LevelObjectState Level0ObjectStates[sizeof(Level0Objects) / sizeof(Level0Objects[0])];

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

static Rect CombineAreas(Rect a, Rect b){
  Rect r;
  r.x0 = (a.x0 < b.x0) ? a.x0 : b.x0;
  r.y0 = (a.y0 < b.y0) ? a.y0 : b.y0;
  r.x1 = (a.x1 > b.x1) ? a.x1 : b.x1;
  r.y1 = (a.y1 > b.y1) ? a.y1 : b.y1;
  return r;
}

static Rect ExpandArea(Rect area, int16_t paddingX, int16_t paddingY){
  area.x0 -= paddingX;
  area.y0 -= paddingY;
  area.x1 += paddingX;
  area.y1 += paddingY;
  return area;
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

static LevelObjectState *ObjectStates(uint8_t levelIndex){
  if(levelIndex == 0){
    return Level0ObjectStates;
  }
  return 0;
}

static ImageData ObjectImage(const LevelObject &object, const LevelObjectState *state){
  if(object.type == LEVEL_OBJECT_LARGE_TREE){
    return LargeTreeImage;
  }
  if(object.type == LEVEL_OBJECT_CHEST){
    uint8_t frame = state ? state->chestFrame : 0;
    if(frame >= CHEST_OPEN_FRAME_COUNT){
      frame = CHEST_OPEN_FRAME_COUNT - 1;
    }
    return ChestOpenFrames[frame];
  }
  return SmallTreeImage;
}

static Rect GetObjectArea(const LevelObject &object, const LevelObjectState *state){
  return GetImageArea(object.x, object.y, ObjectImage(object, state));
}

static void DrawObject(const LevelObject &object, const LevelObjectState *state){
  DrawImageChroma(object.x, object.y, ObjectImage(object, state));
}

static void DrawObjectInsideArea(const LevelObject &object, const LevelObjectState *state, Rect redrawArea){
  ImageData image = ObjectImage(object, state);
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

void ResetLevelObjectStates(uint8_t levelIndex){
  if(levelIndex >= LevelCount){
    return;
  }

  LevelObjectState *states = ObjectStates(levelIndex);
  if(!states){
    return;
  }

  const LevelDefinition &level = Levels[levelIndex];
  for(uint8_t i = 0; i < level.objectCount; i++){
    states[i].chestFrame = 0;
    states[i].chestTick = 0;
    states[i].chestOpened = false;
    states[i].chestAnimating = false;
  }
}

bool TryOpenNearbyChest(uint8_t levelIndex, Rect playerArea, Rect *dirtyArea){
  if(levelIndex >= LevelCount){
    return false;
  }

  LevelObjectState *states = ObjectStates(levelIndex);
  if(!states){
    return false;
  }

  const LevelDefinition &level = Levels[levelIndex];
  for(uint8_t i = 0; i < level.objectCount; i++){
    const LevelObject &object = level.objects[i];
    LevelObjectState &state = states[i];
    if(object.type != LEVEL_OBJECT_CHEST || state.chestOpened){
      continue;
    }

    Rect chestArea = GetObjectArea(object, &state);
    Rect interactionArea = ExpandArea(chestArea, CHEST_INTERACTION_PADDING_X, CHEST_INTERACTION_PADDING_Y);
    if(!AreasOverlap(playerArea, interactionArea)){
      continue;
    }

    Rect previousArea = chestArea;
    state.chestOpened = true;
    state.chestAnimating = true;
    state.chestFrame = 1;
    state.chestTick = 0;

    if(dirtyArea){
      *dirtyArea = CombineAreas(previousArea, GetObjectArea(object, &state));
    }
    return true;
  }

  return false;
}

bool UpdateLevelAnimations(uint8_t levelIndex, Rect *dirtyArea){
  if(levelIndex >= LevelCount){
    return false;
  }

  LevelObjectState *states = ObjectStates(levelIndex);
  if(!states){
    return false;
  }

  const LevelDefinition &level = Levels[levelIndex];
  bool hasDirtyArea = false;
  Rect combinedDirtyArea = {0, 0, 0, 0};

  for(uint8_t i = 0; i < level.objectCount; i++){
    const LevelObject &object = level.objects[i];
    LevelObjectState &state = states[i];
    if(object.type != LEVEL_OBJECT_CHEST || !state.chestAnimating){
      continue;
    }

    state.chestTick++;
    if(state.chestTick < CHEST_OPEN_FRAME_DELAY){
      continue;
    }

    state.chestTick = 0;
    if(state.chestFrame < (CHEST_OPEN_FRAME_COUNT - 1)){
      Rect previousArea = GetObjectArea(object, &state);
      state.chestFrame++;
      Rect changedArea = CombineAreas(previousArea, GetObjectArea(object, &state));
      combinedDirtyArea = hasDirtyArea ? CombineAreas(combinedDirtyArea, changedArea) : changedArea;
      hasDirtyArea = true;
    } else {
      state.chestAnimating = false;
    }
  }

  if(hasDirtyArea && dirtyArea){
    *dirtyArea = combinedDirtyArea;
  }
  return hasDirtyArea;
}

void DrawLevel(uint8_t levelIndex){
  if(levelIndex >= LevelCount){
    return;
  }

  const LevelDefinition &level = Levels[levelIndex];
  LevelObjectState *states = ObjectStates(levelIndex);
  for(uint8_t i = 0; i < level.objectCount; i++){
    DrawObject(level.objects[i], states ? &states[i] : 0);
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
  LevelObjectState *states = ObjectStates(levelIndex);
  for(uint8_t i = 0; i < level.objectCount; i++){
    const LevelObjectState *state = states ? &states[i] : 0;
    if(AreasOverlap(outdatedArea, GetObjectArea(level.objects[i], state))){
      DrawObjectInsideArea(level.objects[i], state, outdatedArea);
    }
  }
  for(uint8_t i = 0; i < level.platformCount; i++){
    if(AreasOverlap(outdatedArea, GetPlatformArea(level.platforms[i]))){
      DrawPlatformRun(level.platforms[i].x, level.platforms[i].y, level.platforms[i].tiles);
    }
  }
}
