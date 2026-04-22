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

// The actual map data lives here in the .cpp file
const uint8_t level_1_map[8][10] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 3, 0, 0},
    {1, 1, 0, 0, 0, 1, 1, 1, 1, 1},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 0, 0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0, 2, 2, 2, 0, 0},
    {3, 0, 0, 0, 0, 1, 1, 1, 0, 3},
    {1, 1, 1, 1, 1, 1, 1, 1, 1, 1} 
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
  {LEVEL_OBJECT_CHEST, 108, 57},
  {LEVEL_OBJECT_CHEST, 76, 37},
  {LEVEL_OBJECT_CHEST, 86, 18},
  {LEVEL_OBJECT_SPIKE_UP, 12, 107},
  {LEVEL_OBJECT_SPIKE_UP, 14, 63},
  {LEVEL_OBJECT_SPIKE_UP, 38, 37},
  {LEVEL_OBJECT_SPIKE_UP, 47, 37},
  {LEVEL_OBJECT_SPIKE_UP, 99, 85},
  {LEVEL_OBJECT_SPIKE_UP, 130, 57},
  {LEVEL_OBJECT_SPIKE_UP, 114, 18}
};

static const LevelPlatform Level1Platforms[] = {
  {0, 130, 5},
  {86, 130, 8},
  {0, 80, 5},
  {14, 55, 4},
  {1, 30, 5},
  {20, 105, 6},
  {75, 105, 6},
  {90, 80, 7},
  {75, 55, 7},
  {90, 30, 7}
};

static const LevelPlatformColumn Level1Columns[] = {
  {65, 85, 9}
};

static const LevelObject Level1Objects[] = {
  // {LEVEL_OBJECT_CHEST, 34, 111},
  // {LEVEL_OBJECT_CHEST, 8, 93},
  // {LEVEL_OBJECT_CHEST, 34, 59},
  // {LEVEL_OBJECT_CHEST, 123, 111},
  // {LEVEL_OBJECT_CHEST, 104, 57},
  // {LEVEL_OBJECT_CHEST, 96, 23},
  // {LEVEL_OBJECT_SPIKE_UP, 4, 111},
  // {LEVEL_OBJECT_SPIKE_UP, 15, 111},
  // {LEVEL_OBJECT_SPIKE_UP, 48, 127},
  // {LEVEL_OBJECT_SPIKE_UP, 58, 127},
  // {LEVEL_OBJECT_SPIKE_UP, 68, 127},
  // {LEVEL_OBJECT_SPIKE_UP, 93, 111},
  // {LEVEL_OBJECT_SPIKE_UP, 104, 111},
  // {LEVEL_OBJECT_SPIKE_UP, 139, 111},
  // {LEVEL_OBJECT_SPIKE_UP, 22, 77},
  // {LEVEL_OBJECT_SPIKE_UP, 31, 77},
  // {LEVEL_OBJECT_SPIKE_UP, 28, 59},
  // {LEVEL_OBJECT_SPIKE_UP, 92, 91},
  // {LEVEL_OBJECT_SPIKE_UP, 114, 73},
  // {LEVEL_OBJECT_SPIKE_UP, 123, 73},
  // {LEVEL_OBJECT_SPIKE_UP, 124, 57},
  // {LEVEL_OBJECT_SPIKE_UP, 134, 57},
  // {LEVEL_OBJECT_SPIKE_UP, 107, 40},
  // {LEVEL_OBJECT_SPIKE_DOWN, 132, 113},
  // {LEVEL_OBJECT_SPIKE_DOWN, 144, 95},
  // {LEVEL_OBJECT_SPIKE_DOWN, 122, 79},
  // {LEVEL_OBJECT_SPIKE_DOWN, 111, 62},
  // {LEVEL_OBJECT_SPIKE_LEFT, 54, 73},
  // {LEVEL_OBJECT_SPIKE_RIGHT, 74, 58}
};

void mainMenu(const char* title, const char* options[], int numOptions, int language){
  (void)language;
  ST7735_SetTextColor(ST7735_WHITE);
  ST7735_FillScreen(ST7735_BLACK);
    
    // Draw Title
    ST7735_SetCursor(3, 2); 
    ST7735_OutString((char*)title);

    // Draw the options dynamically!
    for(int i = 0; i < numOptions; i++) {
        ST7735_SetCursor(7, 5 + (i * 2)); 
        int actualIndex = i + (language * numOptions);
        ST7735_OutString((char*)options[actualIndex]);
    } 
}

void DrawCursor(int oldSelection, int newSelection){
  ST7735_SetCursor(5,5+(oldSelection*2));
  ST7735_OutChar(' ');

  ST7735_SetCursor(5,5+(newSelection*2));
  ST7735_OutChar('>');

}

static LevelObjectState Level0ObjectStates[sizeof(Level0Objects) / sizeof(Level0Objects[0])];
static LevelObjectState Level1ObjectStates[sizeof(Level1Objects) / sizeof(Level1Objects[0])];

const LevelDefinition Levels[] = {
  {
    Level0Platforms,
    sizeof(Level0Platforms) / sizeof(Level0Platforms[0]),
    0,
    0,
    Level0Objects,
    sizeof(Level0Objects) / sizeof(Level0Objects[0])
  },
  {
    Level1Platforms,
    sizeof(Level1Platforms) / sizeof(Level1Platforms[0]),
    Level1Columns,
    sizeof(Level1Columns) / sizeof(Level1Columns[0]),
    Level1Objects,
    sizeof(Level1Objects) / sizeof(Level1Objects[0])
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

static Rect GetPlatformColumnArea(const LevelPlatformColumn &column){
  Rect r;
  r.x0 = column.x;
  r.y0 = column.y - column.tiles * PLATFORM_COLUMN_TILE_HEIGHT + 1;
  r.x1 = column.x + PLATFORM_COLUMN_TILE_WIDTH - 1;
  r.y1 = column.y;
  return r;
}

static int16_t GetPlatformLandingY(const LevelPlatform &platform){
  return platform.y - TILE_SPRITE_HEIGHT;
}

static int16_t GetPlatformColumnLandingY(const LevelPlatformColumn &column){
  return column.y - column.tiles * PLATFORM_COLUMN_TILE_HEIGHT;
}

static bool PlayerFeetOverlapPlatform(Rect playerArea, const LevelPlatform &platform){
  int16_t platformX0 = platform.x;
  int16_t platformX1 = platform.x + platform.tiles * TILE_SPRITE_WIDTH - 1;
  int16_t playerFootX0 = playerArea.x0 + 2;
  int16_t playerFootX1 = playerArea.x1 - 2;
  return (playerFootX0 <= platformX1) && (playerFootX1 >= platformX0);
}

static bool PlayerFeetOverlapPlatformColumn(Rect playerArea, const LevelPlatformColumn &column){
  int16_t columnX0 = column.x;
  int16_t columnX1 = column.x + PLATFORM_COLUMN_TILE_WIDTH - 1;
  int16_t playerFootX0 = playerArea.x0 + 2;
  int16_t playerFootX1 = playerArea.x1 - 2;
  return (playerFootX0 <= columnX1) && (playerFootX1 >= columnX0);
}

static LevelObjectState *ObjectStates(uint8_t levelIndex){
  if(levelIndex == 0){
    return Level0ObjectStates;
  }
  if(levelIndex == 1){
    return Level1ObjectStates;
  }
  return 0;
}

static ImageData ObjectImage(const LevelObject &object, const LevelObjectState *state){
  if(object.type == LEVEL_OBJECT_LARGE_TREE){
    return LargeTreeImage;
  }
  
  // --- THE 4 NEW SPIKES ---
  if(object.type == LEVEL_OBJECT_SPIKE_UP)    return SpikeUpImage; 
  if(object.type == LEVEL_OBJECT_SPIKE_DOWN)  return SpikeDownImage; 
  if(object.type == LEVEL_OBJECT_SPIKE_LEFT)  return SpikeLeftImage; 
  if(object.type == LEVEL_OBJECT_SPIKE_RIGHT) return SpikeRightImage; 
  // ------------------------

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

  for(uint8_t i = 0; i < level.columnCount; i++){
    const LevelPlatformColumn &column = level.columns[i];
    int16_t columnLandingY = GetPlatformColumnLandingY(column);
    bool crossedColumnTop = (previousPlayerArea.y1 <= columnLandingY) && (currentPlayerArea.y1 >= columnLandingY);

    if(crossedColumnTop && PlayerFeetOverlapPlatformColumn(currentPlayerArea, column)){
      if(!foundLanding || columnLandingY < bestLandingY){
        bestLandingY = columnLandingY;
        foundLanding = true;
      }
    }
  }

  if(foundLanding && landingY){
    *landingY = bestLandingY;
  }
  return foundLanding;
}

bool FindPlatformColumnSideCollision(uint8_t levelIndex, Rect previousPlayerArea, Rect currentPlayerArea, int16_t *blockedX){
  if(levelIndex >= LevelCount){
    return false;
  }

  const LevelDefinition &level = Levels[levelIndex];
  int16_t playerWidth = currentPlayerArea.x1 - currentPlayerArea.x0 + 1;
  for(uint8_t i = 0; i < level.columnCount; i++){
    Rect columnArea = GetPlatformColumnArea(level.columns[i]);
    bool verticalOverlap = (currentPlayerArea.y0 <= columnArea.y1) && (currentPlayerArea.y1 >= columnArea.y0);
    if(!verticalOverlap){
      continue;
    }

    bool crossedLeftSide = (previousPlayerArea.x1 < columnArea.x0) && (currentPlayerArea.x1 >= columnArea.x0);
    bool crossedRightSide = (previousPlayerArea.x0 > columnArea.x1) && (currentPlayerArea.x0 <= columnArea.x1);
    if(crossedLeftSide){
      if(blockedX){
        *blockedX = columnArea.x0 - playerWidth;
      }
      return true;
    }
    if(crossedRightSide){
      if(blockedX){
        *blockedX = columnArea.x1 + 1;
      }
      return true;
    }
  }

  return false;
}
bool IsPlayerTouchingSpike(uint8_t levelIndex, Rect playerArea) {
  if(levelIndex >= LevelCount) return false;

  const LevelDefinition &level = Levels[levelIndex];
  for(uint8_t i = 0; i < level.objectCount; i++){
    const LevelObject &object = level.objects[i];
    
    // Check if the object is ANY of the 4 spike types
    if(object.type == LEVEL_OBJECT_SPIKE_UP || 
       object.type == LEVEL_OBJECT_SPIKE_DOWN || 
       object.type == LEVEL_OBJECT_SPIKE_LEFT || 
       object.type == LEVEL_OBJECT_SPIKE_RIGHT) {
       
      Rect spikeArea = GetObjectArea(object, 0);
      
      // Make the spike hitbox a little smaller so it feels fair
      spikeArea = ExpandArea(spikeArea, -2, -2); 
      
      if(AreasOverlap(playerArea, spikeArea)) {
        return true;
      }
    }
  }
  return false;
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
  for(uint8_t i = 0; i < level.columnCount; i++){
    const LevelPlatformColumn &column = level.columns[i];
    if(playerArea.y1 == GetPlatformColumnLandingY(column) && PlayerFeetOverlapPlatformColumn(playerArea, column)){
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
  
  // This automatically draws trees, chests, AND your new spikes!
  for(uint8_t i = 0; i < level.objectCount; i++){
    DrawObject(level.objects[i], states ? &states[i] : 0);
  }
  
  // This draws the platforms
  for(uint8_t i = 0; i < level.platformCount; i++){
    DrawPlatformRun(level.platforms[i].x, level.platforms[i].y, level.platforms[i].tiles);
  }
  for(uint8_t i = 0; i < level.columnCount; i++){
    DrawPlatformColumn(level.columns[i].x, level.columns[i].y, level.columns[i].tiles);
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
  for(uint8_t i = 0; i < level.columnCount; i++){
    if(AreasOverlap(outdatedArea, GetPlatformColumnArea(level.columns[i]))){
      DrawPlatformColumn(level.columns[i].x, level.columns[i].y, level.columns[i].tiles);
    }
  }
}