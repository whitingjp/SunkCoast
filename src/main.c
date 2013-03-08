#include "main.h"

const double _timePerFrame = 1.0f/60.0f;

Point _resolution;
double _totalTime;
double _elapsedTime;

GameData game;

void _render();

int main()
{
  bool running;
  int i;
  
  LOG("Starting game.");
  
  _resolution.x = 80*8;
  _resolution.y = 24*15;
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

  game = game_null_gamedata();
  Entity blah = NULL_ENTITY;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  blah.sprite = spriteData;
  blah.frame = getFrameFromAscii('a', 5);
  game_spawn(&game, blah);
  blah.pos.x = 5;
  blah.player = TRUE;
  blah.frame = getFrameFromAscii('@', 6);
  game_spawn(&game, blah);
  
  LOG("Main loop.");  

  _totalTime = sys_getTime();
  _elapsedTime = 0;
  running = TRUE;
  while(running)
  {
    Color bgCol = {0x1e, 0x23, 0x27, 0xff};
    double time = sys_getTime();
    
    _elapsedTime += time - _totalTime;
    _totalTime = time;
    while(_elapsedTime > _timePerFrame)
    {
      sys_update();
      game_update(&game);
      _elapsedTime -= _timePerFrame;
    }
   
    sys_drawInit(bgCol);
    game_draw(&game);
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
