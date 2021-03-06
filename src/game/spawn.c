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
      out.character = 'o';
      out.name = NAME_BUBBLE;
      out.speed = 1600;
      out.o2 = 1;
      out.strength = 1;
      out.flags = EF_CONTAINSO2;
      out.xp = 0;
      break;
    case ET_WHITEBAIT:
      out.speed = 90;
      out.frame = getFrameFromAscii('w', 1);
      out.character = 'w';
      out.name = NAME_WHITEBAIT;
      out.o2 = 40;
      out.strength = 2;
      out.flags = EF_SENTIENT | EF_SCARES;
      out.xp = 1;
      break;
    case ET_ANEMONE:
      out.speed = 100;
      out.frame = getFrameFromAscii('a', 0);
      out.character = 'a';
      out.name = NAME_ANEMONE;
      out.o2 = 100;
      out.strength = 3;
      out.flags = EF_SENTIENT | EF_STATIONARY;
      out.xp = 2;
      break;
    case ET_TURTLE:
      out.speed = 300;
      out.frame = getFrameFromAscii('t', 3);
      out.character = 't';
      out.name = NAME_TURTLE;
      out.o2 = 100;
      out.strength = 4;
      out.flags = EF_SENTIENT;
      out.xp = 2;
      break;
    case ET_STARFISH:
      out.speed = 140;
      out.frame = getFrameFromAscii('s', 5);
      out.character = 's';
      out.name = NAME_STARFISH;
      out.strength = 3;
      out.o2 = 50;
      out.flags = EF_SENTIENT | EF_SPLITS;
      out.xp = 1;
      break;
    case ET_SEAMONKEY:
      out.speed = 110;
      out.frame = getFrameFromAscii('m', 4);
      out.character = 'm';
      out.name = NAME_SEAMONKEY;
      out.strength = 2;
      out.o2 = 60;
      out.flags = EF_SENTIENT | EF_STEALS | EF_SCARES;
      out.xp = 3;
      break;
    case ET_CUTTLEFISH:
      out.speed = 130;
      out.frame = getFrameFromAscii('c', 0);
      out.character = 'c';
      out.name = NAME_CUTTLEFISH;
      out.strength = 2;
      out.o2 = 70;
      out.flags = EF_SENTIENT | EF_SCARES | EF_INKY;
      out.xp = 4;
      break;
    case ET_PUFFERFISH:
      out.frame = getFrameFromAscii('O', 2);
      out.character = 'O';
      out.name = NAME_PUFFERFISH;
      out.speed = 250;
      out.o2 = 150;
      out.strength = 6;
      out.flags = EF_SENTIENT | EF_CONTAINSO2;
      out.xp = 4;
      break;
    case ET_DOLPHIN:
      out.frame = getFrameFromAscii('d', 2);
      out.character = 'd';
      out.name = NAME_DOLPHIN;
      out.speed = 80;
      out.o2 = 90;
      out.strength = 4;
      out.flags = EF_SENTIENT;
      out.xp = 7;
      break;
    case ET_MERMAID:
      out.frame = getFrameFromAscii('@', 1);
      out.character = '@';
      int rnd = sys_randint(10);
      if(rnd == 0) out.name = NAME_MERMADAME;
      else if(rnd > 6) out.name = NAME_MERMAN;
      else out.name = NAME_MERMAID;
      out.speed = 100;
      out.o2 = 100;
      out.strength = 4;
      out.flags = EF_SENTIENT | EF_TOOLED;
      out.xp = 9;
      break;
    case ET_HYDRA:
      out.frame = getFrameFromAscii('H', 6);
      out.character = 'H';
      out.name = NAME_HYDRA;
      out.speed = 90;
      out.o2 = 400;
      out.strength = 8;
      out.flags = EF_SENTIENT | EF_SPLITS;
      out.xp = 12;
      break;
    case ET_KRAKEN:
      out.frame = getFrameFromAscii('K', 0);
      out.character = 'K';
      out.name = NAME_KRAKEN;
      out.speed = 40;
      out.o2 = 500;
      out.strength = 12;
      out.flags = EF_SENTIENT | EF_SCARES;
      out.xp = 25;
      break;
    case ET_SCUBA:
      out.player = true;
      out.character = 0;
      out.speed = 100;
      out.o2 = 100;
      out.frame = getFrameFromAscii('@', 6);
      out.name = NAME_YOU;
      out.strength = 4;
      out.flags = EF_O2DEPLETES;
      out.xp = 0;
      break;
    default:
      break;
  }
  while(out.xp > game_nextLevel(out.level))
    out.level++;
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
    case IT_DOUBLOON:
      out.subtype = IST_GOLDEN;
      out.frame = getFrameFromAscii('$', out.subtype);
      break;
    default:
      LOG("Trying to spawn invalid item type!");
      out.type = IT_CONCH;
      break;
  }
  return out;
}