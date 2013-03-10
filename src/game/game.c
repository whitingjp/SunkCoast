#include "main.h"

char messages[TILEMAP_HEIGHT][TILEMAP_WIDTH];
int numMessages;

FathomData game_null_fathomdata()
{
  FathomData out;
  int i;
  Entity nullEntity = NULL_ENTITY;
  for(i=0; i<MAX_ENTITIES; i++)
    out.entities[i] = nullEntity;
  out.tileMap = tilemap_generate();
  return out;
}

GameData game_null_gamedata()
{
  GameData out;
  out.current = 0;

  int i;
  for(i=0; i<MAX_FATHOMS; i++)
    out.fathoms[i] = game_null_fathomdata();

  // this isn't quite the right place for this stuff anymore
  numMessages = 0;
  game_addMessage("Welcome to Sunk Coast.");

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

void _game_sortEntities(FathomData* fathom)
{
    qsort((void*)&fathom->entities[0], MAX_ENTITIES, sizeof(Entity), _game_turnCmp);
    int minTurn = fathom->entities[0].turn;
    int i;
    for(i=0; i<MAX_ENTITIES; i++)
    {
      if(!fathom->entities[i].active)
        continue;
      fathom->entities[i].turn -= minTurn;
    }
}

void game_spawn(GameData* game, Entity entity)
{
  FathomData* fathom = &game->fathoms[game->current];
  int i;
  int maxTurn = 0;
  Point spawnPoint = NULL_POINT;
  for(i=0; i<TILEMAP_WIDTH*TILEMAP_HEIGHT; i++)
  {
    spawnPoint.x = sys_randint(TILEMAP_WIDTH);
    spawnPoint.y = sys_randint(TILEMAP_WIDTH);
    if(!tilemap_collides(&fathom->tileMap, spawnPoint))
      break;
  }
  entity.pos = spawnPoint;
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(!fathom->entities[i].active)
      continue;
    if(fathom->entities[i].turn > maxTurn)
      maxTurn = fathom->entities[i].turn;
  }
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(fathom->entities[i].active)
      continue;
    fathom->entities[i] = entity;
    fathom->entities[i].active = true;
    fathom->entities[i].turn = maxTurn+1;
    _game_sortEntities(fathom);
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

void _draw_hud(const GameData* game, Entity e, Point offset)
{
  char string[TILEMAP_WIDTH];
  snprintf(string, TILEMAP_WIDTH, "     O2: %d/%d", e.o2, e.maxo2);
  sys_drawString(offset, string, TILEMAP_WIDTH, 2);
  snprintf(string, TILEMAP_WIDTH, "fathoms: %d", (game->current+1)*10);
  offset.y++;
  sys_drawString(offset, string, TILEMAP_WIDTH, 2);
}


int _get_playerIndex(const FathomData* fathom)
{
  int i;
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(!fathom->entities[i].active)
      continue;
    if(!fathom->entities[i].player)
      continue;
    return i;
  }
  return -1;
}

void game_draw(const GameData* game, Point offset)
{
  const FathomData* fathom = &game->fathoms[game->current];
  int i;
  tilemap_draw(fathom->tileMap, offset);
  for(i=0; i<MAX_ENTITIES; i++)
  {
    const Entity* e = &fathom->entities[i];
    if(!e->active)
      continue;
    if(!tilemap_visible(&fathom->tileMap, e->pos))
      continue;
    Point drawPos = pointAddPoint(offset, e->pos);
    sys_drawSprite(e->sprite, e->frame, drawPos);   
  }
  if(sys_inputDown(INPUT_A))
    _draw_route(&fathom->tileMap, fathom->entities[0].pos, fathom->entities[1].pos, fathom->entities[0].sprite, fathom->entities[0].frame);

  Point messagePos = NULL_POINT;
  for(i=0; i<numMessages; i++)
  {    
    sys_drawString(messagePos, messages[i], TILEMAP_WIDTH, 1);
    messagePos.y++;
  }
  int playerIndex = _get_playerIndex(fathom);
  if(playerIndex != -1)
  {
    Point pos = offset;
    pos.y += TILEMAP_HEIGHT;
    _draw_hud(game, fathom->entities[playerIndex], pos);
  }
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

Point _getAiInput(const FathomData* fathom, Entity* e)
{
  Point pos = e->pos;
  Point move = NULL_POINT;
  if(e->sentient)
  {
    int player =  _get_playerIndex(fathom);
    bool visible = tilemap_visible(&fathom->tileMap, pos);
    if(player != -1 && visible)
      move = _get_aiPath(&fathom->tileMap, pos, fathom->entities[player].pos);
  }
  if(move.x == 0 && move.y == 0)
  {
    int dir = sys_randint(4);
    move = directionToPoint(dir);
  }
  return move;
}

void _do_turn(FathomData* fathom, Entity* e, Point move)
{
    Entity nullEntity = NULL_ENTITY;
    if(e->player)
      numMessages = 0;    
    Point newPoint = pointAddPoint(e->pos, move);
    bool isWall = tilemap_collides(&fathom->tileMap, newPoint);
    bool isEntity = false;
    int i;
    for(i=0; i<MAX_ENTITIES; i++)
    {
      if(i==0)
        continue;
      Entity* victim = &fathom->entities[i];
      if(!victim->active)
        continue;
      if(newPoint.x != victim->pos.x)
        continue;
      if(newPoint.y != victim->pos.y)
        continue;
      
      int amount = sys_randint(e->strength);
      victim->o2 -= amount*10;
      if(amount == 0)
        game_addMessage("%s missed %s", e->name, victim->name);
      else
        game_addMessage("%s hit %s", e->name, victim->name);
      if(victim->o2 <= 0)
      {
        if(victim->containso2)
        {
          int boost = (sys_randint(3)+sys_randint(3)+2)*10;
          e->o2 = min(e->o2 + boost, e->maxo2);
        }
        *victim = nullEntity;
      }
      isEntity = true;
      break;
    }

    if(!isWall && !isEntity)
      e->pos = newPoint;

    if(e->o2depletes)
    {
      e->o2timer++;
      if(e->o2timer >= 5)
      {
        e->o2--;
        e->o2timer = 0;
        if(e->o2 <= 0)
        {
          game_addMessage("%s drowned", e->name);
          *e = nullEntity;
        }
      }
    }
    e->turn += e->speed+sys_randint(e->speed);
}

void _game_dive(GameData* game, int entityIndex, int depth)
{ 
  Entity nullEntity = NULL_ENTITY; 
  FathomData* currentFathom = &game->fathoms[game->current];
  int newFathomIndex = game->current + depth;
  Entity e = currentFathom->entities[entityIndex];
  if(newFathomIndex < 0)
  {
    game_addMessage("%s are on the surface", e.name);
    return;
  }
  if(newFathomIndex >= MAX_FATHOMS)
  {
    game_addMessage("%s are on the bottom of the ocean", e.name);
    return;
  }  

  currentFathom->entities[entityIndex] = nullEntity;
  game->current += depth;
  game_spawn(game, e);
}

void _game_recalcFov(FathomData* fathom)
{
  int i;
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(!fathom->entities[i].player)
      continue;
    tilemap_recalcFov(&fathom->tileMap, fathom->entities[i].pos);
  }
}

void game_update(GameData* game)
{
  FathomData* fathom = &game->fathoms[game->current];
  Entity* e = &fathom->entities[0];
  Point move = NULL_POINT;
  if(!e->active)
    return;
  if(e->player)
  {
    move = _getInput();
    if(sys_inputPressed(INPUT_DIVE))
    {
      _game_dive(game, 0, 1);
      _game_recalcFov(&game->fathoms[game->current]);
      return;
    }
    if(sys_inputPressed(INPUT_RISE))
    {
      _game_dive(game, 0, -1);
      _game_recalcFov(&game->fathoms[game->current]);
      return;
    }
  }
  else
  {
    move = _getAiInput(fathom, e);
  }

  if(move.x != 0 || move.y != 0)
  {
    _do_turn(fathom, e, move);

    _game_sortEntities(fathom);
    _game_recalcFov(fathom);
  }
}

void game_addMessage(const char *str, ...)
{
  if(numMessages >= TILEMAP_HEIGHT)
  {
    LOG("Not enough space for messages.");
    return;
  }
  va_list args;
  va_start(args, str);
  vsnprintf(messages[numMessages], TILEMAP_WIDTH, str, args);
  numMessages++;
}
