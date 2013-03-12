#include "main.h"

Entity spawn_entity(EntityType type)
{
  Entity out = NULL_ENTITY;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  out.sprite = spriteData;
  switch(type)
  {
    case ET_BUBBLE:
      out.frame = getFrameFromAscii('o', 2);
      out.name = "bubble";
      out.speed = 1600;
      out.o2 = 1;
      out.strength = 1;
      out.flags = EF_CONTAINSO2;
      break;
    case ET_WHITEBAIT:
      out.speed = 90;
      out.frame = getFrameFromAscii('w', 1);
      out.name = "whitebait";
      out.o2 = 40;
      out.strength = 2;
      out.flags = EF_SENTIENT | EF_SCARES;
      break;
    case ET_TURTLE:
      out.speed = 300;
      out.frame = getFrameFromAscii('t', 3);
      out.name = "turtle";
      out.o2 = 100;
      out.strength = 4;
      out.flags = EF_SENTIENT;
      break;
    case ET_STARFISH:
      out.speed = 140;
      out.frame = getFrameFromAscii('s', 5);      
      out.name = "starfish";
      out.strength = 3;
      out.o2 = 50;
      out.flags = EF_SENTIENT | EF_SPLITS;
      break;
    case ET_SEAMONKEY:
      out.speed = 110;
      out.frame = getFrameFromAscii('m', 4);
      out.name = "sea monkey";
      out.strength = 2;
      out.o2 = 60;
      out.flags = EF_SENTIENT | EF_STEALS | EF_SCARES;
      break;   
    case ET_PUFFERFISH:
      out.frame = getFrameFromAscii('O', 2);
      out.name = "pufferfish";
      out.speed = 250;
      out.o2 = 150;
      out.strength = 6;
      out.flags = EF_SENTIENT | EF_CONTAINSO2;
      break;
    case ET_HYDRA:
      out.frame = getFrameFromAscii('H', 6);
      out.name = "hydra";
      out.speed = 90;
      out.o2 = 400;
      out.strength = 8;
      out.flags = EF_SENTIENT | EF_SPLITS;
      break;
    case ET_SCUBA:
      out.player = true;
      out.speed = 100;
      out.frame = getFrameFromAscii('@', 6);
      out.name = "you";
      out.strength = 4;
      out.flags = EF_O2DEPLETES;
      break;
  }
  return out;
}

Item spawn_item(const GameData* game, ItemType type)
{
  Item out = NULL_ITEM;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  out.sprite = spriteData;
  out.type = type;  
  switch(out.type)
  {
    case IT_CONCH:     
      out.conchSubtype = sys_randint(CONCH_MAX);
      out.subtype = game->conchTypes[out.conchSubtype];
      out.frame = getFrameFromAscii('&', out.subtype);
      break;
    case IT_CHARM:      
      out.charmSubtype = sys_randint(CHARM_MAX);
      out.subtype = game->charmTypes[out.charmSubtype];
      out.frame = getFrameFromAscii('=', out.subtype);
      break;
    default:
      LOG("Trying to spawn invalid item type!");
      out.type = IT_CONCH;
      break;
  }
  return out;
}