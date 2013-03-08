#include "main.h"

const double _timePerFrame = 1.0f/60.0f;

Point _resolution;
World _world;
double _totalTime;
double _elapsedTime;

void _render();

int main()
{
  bool running;
  int i;
  
  LOG("Starting game.");  
  
  _resolution.x = 80*8;
  _resolution.y = 24*15;
  sys_init(_resolution, 4);

  _world = world_load();
  
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
  
  LOG("Main loop.");  

  _totalTime = sys_getTime();
  _elapsedTime = 0;
  running = TRUE;
  while(running)
  {
    Color bgCol = {0x1e, 0x23, 0x27, 0xff};
    //TileMap tileMap = NULL_TILEMAP;
    double time = sys_getTime();
    
    _elapsedTime += time - _totalTime;
    _totalTime = time;
    while(_elapsedTime > _timePerFrame)
    {
      sys_update();
      _elapsedTime -= _timePerFrame;
    }
   
    sys_drawInit(bgCol);
    //tileMap = _world.screens[_gameData.screen].tileMap;
    //tilemap_render(tileMap, _gameData.camera.offset);
    SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
    Point pos = {4,1};
    Point frame = {1,9};
    sys_drawSprite(spriteData, frame, pos);
    sys_drawFinish();

    if(sys_shouldClose())
      running = FALSE;
  }
  
  LOG("Closing.");
  sys_close();
  return EXIT_SUCCESS;
}

Point main_getResolution()
{
  return _resolution;
}
