#include "main.h"

GameData game_null_gamedata()
{
  GameData out;
  int i;
  Entity nullEntity = NULL_ENTITY;
  for(i=0; i<MAX_ENTITIES; i++)
    out.entities[i] = nullEntity;
  out.tileMap = tilemap_generate();
  return out;
}

int _game_turnCmp(const void* a, const void* b)
{
  Entity* eA = (Entity*)a;
  Entity* eB = (Entity*)b;
  if (eA->turn == eB->turn)
    return 0;
  else if (eA->turn < eB->turn)
    return -1;
  else
    return 1;
}

void _game_sortEntities(GameData* game)
{
    qsort((void*)&game->entities[0], MAX_ENTITIES, sizeof(Entity), _game_turnCmp);
    int minTurn = game->entities[0].turn;
    int i;
    for(i=0; i<MAX_ENTITIES; i++)
    {
      if(!game->entities[i].active)
        continue;
      game->entities[i].turn -= minTurn;
    }
}

void game_spawn(GameData* game, Entity entity)
{
  int i;
  int maxTurn = 0;
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(!game->entities[i].active)
      continue;
    if(game->entities[i].turn > maxTurn)
      maxTurn = game->entities[i].turn;
  }
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(game->entities[i].active)
      continue;
    game->entities[i] = entity;
    game->entities[i].active = true;
    game->entities[i].turn = maxTurn+1;
    _game_sortEntities(game);
    return;
  }
  LOG("Couldn't find a free entity space, not spawning.");
}

void game_draw(const GameData* game)
{
  int i;
  Point nullPoint = NULL_POINT;
  tilemap_draw(game->tileMap, nullPoint);
  for(i=0; i<MAX_ENTITIES; i++)
  {
    const Entity* e = &game->entities[i];
    if(!e->active)
      continue;
    sys_drawSprite(e->sprite, e->frame, e->pos);
  }
}

Point _get_input()
{
  Point move = NULL_POINT;
  if(sys_inputPressed(INPUT_UP))
    move.y--;
  if(sys_inputPressed(INPUT_RIGHT))
    move.x++;
  if(sys_inputPressed(INPUT_DOWN))
    move.y++;
  if(sys_inputPressed(INPUT_LEFT))
    move.x--;
  return move;
}

void game_update(GameData* game)
{
  Point move = NULL_POINT;
  if(game->entities[0].player)
    move = _get_input();
  else
    move.y = 1;
  if(move.x != 0 || move.y != 0)
  {    
    Point newPoint = pointAddPoint(game->entities[0].pos, move);
    if(!tilemap_collides(&game->tileMap, newPoint))
      game->entities[0].pos = newPoint;
    game->entities[0].turn += 100;
    _game_sortEntities(game);
    tilemap_recalcFov(&game->tileMap, game->entities[0].pos);
  }
}