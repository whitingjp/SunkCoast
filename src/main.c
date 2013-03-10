#include "main.h"

const char* line0 = "   _____             __      ______                 __  ";
const char* line1 = "  / ___/__  ______  / /__   / ____/___  ____ ______/ /_ ";
const char* line2 = "  \\__ \\/ / / / __ \\/ //_/  / /   / __ \\/ __ `/ ___/ __/ ";
const char* line3 = " ___/ / /_/ / / / / ,<    / /___/ /_/ / /_/ (__  ) /_  ";
const char* line4 = "/____/\\__,_/_/ /_/_/|_|   \\____/\\____/\\__,_/____/\\__/  ";

const double _timePerFrame = 1.0f/60.0f;

Point _resolution;
double _totalTime;
double _elapsedTime;

bool preGame = true;
GameData game;

void drawBanner()
{
  int i;
  int j;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  for(i=0; i<5; i++)
  {
    const char *str = NULL;
    switch(i)
    {
      case 0: str = line0; break;
      case 1: str = line1; break;
      case 2: str = line2; break;
      case 3: str = line3; break;
      case 4: str = line4; break;
    }
    for(j=0; j<55; j++)
    {
      const char c = str[j];
      Point frame = getFrameFromAscii(c, i+2);
      Point pos = NULL_POINT;
      pos.x = (TILEMAP_WIDTH-55)/2+j;
      pos.y = (TILEMAP_HEIGHT-5)/2+i;
      sys_drawSprite(spriteData, frame, pos);
    }    
  }
}

int main()
{
  bool running;
  int i;
  
  LOG("Starting game.");
  
  _resolution.x = TILEMAP_WIDTH*8;
  _resolution.y = (TILEMAP_HEIGHT+3)*15;
  sys_init(_resolution, 4);
  
  for(i=0; i<IMAGE_MAX; i++)
  {
    switch(i)
    {
      case IMAGE_FONT:
        sys_loadImage("data/graphics/font.png");
        break;
      default:
        LOG("Missing image load for ImageID:%d", i);
        exit(EXIT_FAILURE);
    }
  }

  game = NULL_GAMEDATA;
  LOG("Sizeof GameData %dkb.", sizeof(GameData)/1024);
  
  LOG("Main loop.");  

  _totalTime = sys_getTime();
  _elapsedTime = 0;
  running = true;
  while(running)
  {
    Color bgCol = {0x1e, 0x23, 0x27, 0xff};
    double time = sys_getTime();
    
    _elapsedTime += time - _totalTime;
    _totalTime = time;
    while(_elapsedTime > _timePerFrame)
    {
      sys_update();
      if(preGame)
      {
        if(sys_inputPressed(INPUT_ANY))
          preGame = false;
      }
      else
      {
        while(!game_update(&game)) {}
      }
      _elapsedTime -= _timePerFrame;
    }
    
    Point offset = NULL_POINT;
    offset.y = 1;
    sys_drawInit(bgCol);
    if(preGame)
      drawBanner();
    else
      game_draw(&game, offset);
    sys_drawFinish();

    if(sys_shouldClose())
      running = false;
  }
  
  LOG("Closing.");
  sys_close();
  return EXIT_SUCCESS;
}

Point main_getResolution()
{
  return _resolution;
}
