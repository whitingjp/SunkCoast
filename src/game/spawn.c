#include "main.h"

Entity spawn_create(SpawnType type)
{
  Entity out = NULL_ENTITY;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  out.sprite = spriteData;
  switch(type)
  {
    case ST_SCUBA:
      out.player = true;
      out.speed = 100;
      out.frame = getFrameFromAscii('@', 6);
      out.name = "you";
      out.o2depletes = true;
      break;
    case ST_STARFISH:
      out.speed = 140;
      out.frame = getFrameFromAscii('s', 5);      
      out.name = "starfish";
      out.sentient = true;
      break;
    case ST_BUBBLE:
      out.frame = getFrameFromAscii('o', 2);
      out.name = "bubble";
      out.speed = 400;
      out.o2 = 1;
      out.containso2 = true;
      break;
  }
  return out;
}