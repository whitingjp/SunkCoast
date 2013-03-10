#include "main.h"

Entity spawn_entity(EntityType type)
{
  Entity out = NULL_ENTITY;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  out.sprite = spriteData;
  switch(type)
  {
    case ET_SCUBA:
      out.player = true;
      out.speed = 100;
      out.frame = getFrameFromAscii('@', 6);
      out.name = "you";
      out.o2depletes = true;
      break;
    case ET_STARFISH:
      out.speed = 140;
      out.frame = getFrameFromAscii('s', 5);      
      out.name = "starfish";
      out.sentient = true;
      break;
    case ET_BUBBLE:
      out.frame = getFrameFromAscii('o', 2);
      out.name = "bubble";
      out.speed = 400;
      out.o2 = 1;
      out.containso2 = true;
      break;
  }
  return out;
}

Item spawn_item(ItemType type)
{
  Item out = NULL_ITEM;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  out.sprite = spriteData;
  out.type = type;
  out.subtype = sys_randint(7);
  switch(out.type)
  {
    case IT_CONCH:
      out.frame = getFrameFromAscii('&', out.subtype);
      break;
    case IT_CHARM:
      out.frame = getFrameFromAscii('=', out.subtype);
      break;
    default:
      LOG("Trying to spawn invalid item type!");
      out.type = IT_CONCH;
      break;
  }
  return out;
}