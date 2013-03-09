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

const TileMap* _aStartTileMap;

uint8_t _get_map_cost (const uint32_t x, const uint32_t y)
{
  Point p = NULL_POINT;
  p.x = x;
  p.y = y;
  if(tilemap_collides(_aStartTileMap, p))
    return COST_BLOCKED;
  else
    return 1;
}

void _draw_route(const TileMap* tileMap, Point start, Point end, SpriteData sprite, Point frame)
{
  _aStartTileMap = tileMap;
  astar_t *as = astar_new(TILEMAP_WIDTH, TILEMAP_HEIGHT, _get_map_cost, NULL);
  astar_set_origin (as, 0, 0);
  astar_set_steering_penalty (as, 0);
  astar_set_movement_mode (as, DIR_CARDINAL);
  astar_run (as, start.x, start.y, end.x, end.y);
  if(astar_have_route(as))
  {
    direction_t * directions, * dir;
    int i, num_steps;
    num_steps = astar_get_directions (as, &directions);    
    Point cur = start;
    dir = directions;
    for (i = 0; i < num_steps; i++, dir++)
    {
      sys_drawSprite(sprite, frame, cur);
      cur.x += astar_get_dx(as, *dir);
      cur.y += astar_get_dy(as, *dir);
    }
    astar_free_directions (directions);
  }
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
    if(!tilemap_visible(&game->tileMap, e->pos))
      continue;
    sys_drawSprite(e->sprite, e->frame, e->pos);    
  }
  if(sys_inputDown(INPUT_A))
    _draw_route(&game->tileMap, game->entities[0].pos, game->entities[1].pos, game->entities[0].sprite, game->entities[0].frame);
}

Point _getInput()
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

Point _get_aiPath(const TileMap* tileMap, Point start, Point end)
{
  Point move = NULL_POINT;
  _aStartTileMap = tileMap;
  astar_t *as = astar_new(TILEMAP_WIDTH, TILEMAP_HEIGHT, _get_map_cost, NULL);
  astar_set_origin (as, 0, 0);
  astar_set_steering_penalty (as, 0);
  astar_set_movement_mode (as, DIR_CARDINAL);
  astar_run (as, start.x, start.y, end.x, end.y);
  if(astar_have_route(as))
  {
    direction_t * directions, * dir;
    astar_get_directions (as, &directions);    
    dir = directions;
    move.x += astar_get_dx(as, *dir);
    move.y += astar_get_dy(as, *dir);
    astar_free_directions (directions);
  }
  return move;
}

int _get_playerIndex(GameData* game)
{
  int i;
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(!game->entities[i].active)
      continue;
    if(!game->entities[i].player)
      continue;
    return i;
  }
  return -1;
}

Point _getAiInput(GameData* game)
{
  Point pos = game->entities[0].pos;
  Point move = NULL_POINT;
  int player =  _get_playerIndex(game);
  bool visible = tilemap_visible(&game->tileMap, pos);
  if(player != -1 && visible)
    move = _get_aiPath(&game->tileMap, pos, game->entities[player].pos);
  if(move.x == 0 && move.y == 0)
  {
    int dir = sys_randint(4);
    move = directionToPoint(dir);
  }
  return move;
}

void game_update(GameData* game)
{
  int i;
  Point move = NULL_POINT;
  if(!game->entities[0].active)
    return;
  if(game->entities[0].player)
    move = _getInput();
  else
    move = _getAiInput(game);

  if(move.x != 0 || move.y != 0)
  {
    Point newPoint = pointAddPoint(game->entities[0].pos, move);
    bool isWall = tilemap_collides(&game->tileMap, newPoint);
    bool isEntity = false;
    for(i=0; i<MAX_ENTITIES; i++)
    {
      if(i==0)
        continue;
      if(!game->entities[i].active)
        continue;
      if(newPoint.x != game->entities[i].pos.x)
        continue;
      if(newPoint.y != game->entities[i].pos.y)
        continue;
      LOG("%d hit %d!", 0, i);
      isEntity = true;
    }
    if(!isWall && !isEntity)
      game->entities[0].pos = newPoint;

    game->entities[0].turn += game->entities[0].speed;
    _game_sortEntities(game);
    for(i=0; i<MAX_ENTITIES; i++)
    {
      if(!game->entities[i].player)
        continue;
      tilemap_recalcFov(&game->tileMap, game->entities[i].pos);
    }
    
  }
}