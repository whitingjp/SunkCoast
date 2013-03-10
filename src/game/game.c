#include "main.h"

char messages[TILEMAP_HEIGHT][TILEMAP_WIDTH];
int numMessages;
bool midDrop;
bool midUse;
bool midFire;
int fireIndex;

Entity game_null_entity()
{
  SpriteData nullSpriteData = NULL_SPRITEDATA;
  Point nullPoint = NULL_POINT;
  Item nullItem = NULL_ITEM;

  Entity out;
  out.active = false;
  out.sprite = nullSpriteData;
  out.frame = nullPoint;
  out.pos = nullPoint;
  out.turn = INT_MAX;
  out.player = false;
  out.speed = 100;
  out.o2 = 100;
  out.maxo2 = 100;
  out.o2timer = 0;
  out.o2depletes = false;
  out.mana = 50;
  out.maxMana = 100;
  out.strength = 4;
  out.name = NULL;
  out.sentient = false;
  out.containso2 = false;

  int i;
  for(i=0; i<MAX_INVENTORY; i++)
    out.inventory[i] = nullItem;
  return out;
}

FathomData game_null_fathomdata()
{
  FathomData out;
  int i;
  Entity nullEntity = NULL_ENTITY;
  for(i=0; i<MAX_ENTITIES; i++)
    out.entities[i] = nullEntity;

  Item nullItem = NULL_ITEM;
  for(i=0; i<MAX_ITEMS; i++)
    out.items[i] = nullItem;

  out.tileMap = tilemap_generate();

  for(i=0; i<4; i++)
  {
    game_spawn(&out, spawn_entity(ET_STARFISH));
    game_spawn(&out, spawn_entity(ET_BUBBLE));
  }
  return out;
}

GameData game_null_gamedata()
{
  GameData out;
  out.current = 0;

  item_shuffleTypes((int*)out.charmTypes, CHARM_MAX);
  item_shuffleTypes((int*)out.conchTypes, CONCH_MAX);

  int i;
  for(i=0; i<MAX_FATHOMS; i++)
  {
    out.fathoms[i] = game_null_fathomdata();
    game_place(&out.fathoms[i], spawn_item(&out, IT_CONCH));
    game_place(&out.fathoms[i], spawn_item(&out, IT_CHARM));
  }
  game_spawn(&out.fathoms[0], spawn_entity(ET_SCUBA));    

  // this isn't quite the right place for this stuff anymore
  numMessages = 0;
  game_addGlobalMessage("Welcome to Sunk Coast.");
  midDrop = false;

  return out;
}

bool game_hasCharm(const Entity *e, CharmSubType charm)
{
  int i;
  for(i=0; i<MAX_ITEMS; i++)
  {
    if(e->inventory[i].type != IT_CHARM)
      continue;
    if(e->inventory[i].charmSubtype != charm)
      continue;
    if(!e->inventory[i].worn)
      continue;
    return true;
  }
  return false;
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

Point _game_getSpawnPoint(const FathomData* fathom)
{
  Point spawnPoint = NULL_POINT;
  int i;
  for(i=0; i<TILEMAP_WIDTH*TILEMAP_HEIGHT; i++)
  {
    spawnPoint.x = sys_randint(TILEMAP_WIDTH);
    spawnPoint.y = sys_randint(TILEMAP_WIDTH);
    if(!tilemap_collides(&fathom->tileMap, spawnPoint))
      break;
  }
  return spawnPoint;
}

void game_place(FathomData* fathom, Item item)
{
  item.pos = _game_getSpawnPoint(fathom);
  item.active = true;

  int i;  
  for(i=0; i<MAX_ITEMS; i++)
  {
    if(fathom->items[i].active)
      continue;
    fathom->items[i] = item;
    return;
  }
  LOG("Couldn't find a free item space, not placing.");
}

void game_spawnAt(FathomData* fathom, Entity entity, Point pos)
{
  if(pos.x > 0 && pos.y > 0 && !tilemap_collides(&fathom->tileMap, pos))
    entity.pos = pos;
  else
    entity.pos = _game_getSpawnPoint(fathom);

  int i;
  int maxTurn = 0;
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(!fathom->entities[i].active)
      continue;
    if(fathom->entities[i].turn > maxTurn)
      maxTurn = fathom->entities[i].turn;
  }
  entity.active = true;
  entity.turn = maxTurn+1;

  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(fathom->entities[i].active)
      continue;
    fathom->entities[i] = entity;
    _game_sortEntities(fathom);
    return;
  }
  LOG("Couldn't find a free entity space, not spawning.");
}
void game_spawn(FathomData* fathom, Entity entity)
{
  game_spawnAt(fathom, entity, _game_getSpawnPoint(fathom));
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
  int o2col = 2;
  if(e.o2 < e.maxo2/4)
    o2col = 6;
  snprintf(string, TILEMAP_WIDTH, "   O2: %3d/%3d", e.o2, e.maxo2);
  sys_drawString(offset, string, TILEMAP_WIDTH, o2col);

  Point manaPos = offset;
  manaPos.y++;
  int manacol = 2;
  if(e.mana < e.maxMana/4)
    manacol = 6;
  snprintf(string, TILEMAP_WIDTH, " mana: %3d/%3d", e.mana, e.maxMana);
  sys_drawString(manaPos, string, TILEMAP_WIDTH, manacol);

  Point fathomPos = offset;
  fathomPos.x = TILEMAP_WIDTH-12;
  fathomPos.y++;
  snprintf(string, TILEMAP_WIDTH, "fathoms: %d", (game->current+1)*10);
  sys_drawString(fathomPos, string, TILEMAP_WIDTH, 2);

  Point inventoryPos = offset;
  inventoryPos.x = TILEMAP_WIDTH-MAX_INVENTORY*4;
  int i;
  for(i=0; i<MAX_INVENTORY; i++)
  {
    Item item = e.inventory[i];
    int itemcol = 2;
    if(item.worn)
      itemcol = 4;
    snprintf(string, TILEMAP_WIDTH, "%d:", i+1);
    sys_drawString(inventoryPos, string, TILEMAP_WIDTH, itemcol);
    inventoryPos.x += 2;
    sys_drawSprite(item.sprite, item.frame, inventoryPos);
    inventoryPos.x += 2;
  }
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
  for(i=0; i<MAX_ITEMS; i++)
  {
    const Item* item = &fathom->items[i];
    if(!item->active)
      continue;
    if(!tilemap_visible(&fathom->tileMap, item->pos))
      continue;
    Point drawPos = pointAddPoint(offset, item->pos);
    sys_drawSprite(item->sprite, item->frame, drawPos);
  }
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
  //if(sys_inputDown(INPUT_A))
  //  _draw_route(&fathom->tileMap, fathom->entities[0].pos, fathom->entities[1].pos, fathom->entities[0].sprite, fathom->entities[0].frame);

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

void _do_move(FathomData* fathom, Entity* e, Point move)
{
  Entity nullEntity = NULL_ENTITY;
  if(game_hasCharm(e, CHARM_HASTE))
  {
    if(sys_randint(15)==0)
    {
      Direction d = sys_randint(4);
      move = directionToPoint(d);
    }
  }
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
      game_addMessage(fathom, newPoint, "%s missed %s", e->name, victim->name);
    else
      game_addMessage(fathom, newPoint, "%s hit %s", e->name, victim->name);
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
}

void _do_pickup(FathomData* fathom, Entity* e)
{
  Item nullItem = NULL_ITEM;

  int i;
  for(i=0; i<MAX_ITEMS; i++)
  {
    Item* item = &fathom->items[i];
    if(!item->active)
      continue;
    if(item->pos.x != e->pos.x || item->pos.y != e->pos.y)
      continue;

    int j;
    for(j=0; j<MAX_INVENTORY; j++)
    {
      if(!e->inventory[j].active)
      {
        e->inventory[j] = *item;
        game_addMessage(fathom, e->pos, "%s picked up %s %s", e->name, item_subtypeDescription(item->subtype), item_typeName(item->type));
        *item = nullItem;
        break;
      }
    }
    if(j==MAX_INVENTORY)
      game_addMessage(fathom, e->pos, "%s have no room for %s %s", e->name, item_subtypeDescription(item->subtype), item_typeName(item->type));
  }
}

bool _do_drop(FathomData* fathom, Entity* e, int index)
{
  Item nullItem = NULL_ITEM;
  Item item = e->inventory[index];
  item.pos = e->pos;
  if(!e->inventory[index].active)
  {
    LOG("Dropping non existent item");
    return false;
  }
  int empty = -1;
  int i;  
  for(i=0; i<MAX_ITEMS; i++)
  {
    if(!fathom->items[i].active)
    {
      empty = i;
      break;
    }
  }
  if(empty == -1)
  {
    game_addMessage(fathom, e->pos, "%s can't drop %s %s, too cluttered", e->name, item_subtypeDescription(item.subtype), item_typeName(item.type));
    return false;
  }
  item.worn = false;
  fathom->items[empty] = item;  
  e->inventory[index] = nullItem;
  game_addMessage(fathom, e->pos, "%s dropped %s %s", e->name, item_subtypeDescription(item.subtype), item_typeName(item.type));
  return true;
}

bool _do_use(FathomData* fathom, Entity* e, int index)
{
  Item* item = &e->inventory[index];
  if(!item->active)
  {
    LOG("Using non existent item");
    return false;
  }
  if(item->type == IT_CHARM)
  {
    item->worn = !item->worn;
    if(item->worn)
      game_addMessage(fathom, e->pos, "%s put on %s %s", e->name, item_subtypeDescription(item->subtype), item_typeName(item->type));
    else
      game_addMessage(fathom, e->pos, "%s took off %s %s", e->name, item_subtypeDescription(item->subtype), item_typeName(item->type));
    return true;
  }
  if(item->type == IT_CONCH)
  {
    game_addGlobalMessage("Point %s %s in which direction?", item_subtypeDescription(item->subtype), item_typeName(item->type));
    midFire = true;
    fireIndex = index;
    return false;
  }
  return false;
}

void _do_fire(FathomData* fathom, Entity* e, int index, Direction direction)
{
  Item* item = &e->inventory[index];
  int manaCost = 20;
  manaCost += sys_randint(manaCost/2);
  if(e->mana > manaCost)
  {
    Point vector = directionToPoint(direction);
    Point pos = pointAddPoint(e->pos, vector);
    e->mana -= manaCost;
    int distance = 3 + sys_randint(3);
    int i;

    int spawnType = sys_randint(ET_MAX_ENEMY);
    Tile nullTile = NULL_TILE;
    for(i=0; i<distance; i++)
    {
      int index = tilemap_indexFromTilePosition(&fathom->tileMap, pos);
      switch(item->conchSubtype)
      {
        case CONCH_MONSTER:
          game_spawnAt(fathom, spawn_entity(spawnType), pos);
          break;
        case CONCH_DIG:          
          if(index != -1)
            fathom->tileMap.tiles[index] = nullTile;
          break;
        default:
          break;
      }      
      pos = pointAddPoint(pos, vector);
    }
  } else
  {
    Entity nullEntity = NULL_ENTITY;
    e->mana = 0;
    if(e->player)
      game_addGlobalMessage("Not enough mana. Your mind screams.");
    e->o2 -= 15+sys_randint(15);
    if(e->o2 < 0)
    {
      game_addMessage(fathom, e->pos, "%s drowned", e->name);
      *e = nullEntity;
    }
  }

  game_addMessage(fathom, e->pos, "%s fired %s %s in %d", e->name, item_subtypeDescription(item->subtype), item_typeName(item->type), direction);
}

void _do_turn(FathomData* fathom, Entity* e)
{
  Entity nullEntity = NULL_ENTITY;
  if(e->o2depletes)
  {
    e->o2timer++;
    if(e->o2timer >= 5)
    {
      e->o2--;
      e->o2timer = 0;
      if(e->o2 <= 0)
      {
        game_addMessage(fathom, e->pos, "%s drowned", e->name);
        *e = nullEntity;
      }
    }
  }

  if(sys_randint(4) == 0)
    e->mana += sys_randint(e->maxMana)/10;
  if(e->mana > e->maxMana)
    e->mana = e->maxMana;

  if(e->player)
  {
    int i;
    for(i=0; i<MAX_ITEMS; i++)
    {
      Item* item = &fathom->items[i];
      if(!item->active)
        continue;
      if(e->pos.x != item->pos.x || e->pos.y != item->pos.y)
        continue;
      game_addMessage(fathom, item->pos, "%s see a %s %s", e->name, item_subtypeDescription(item->subtype), item_typeName(item->type));
    }
  }

  int turnAdd = e->speed+sys_randint(e->speed);
  if(game_hasCharm(e, CHARM_HASTE))
    turnAdd = (turnAdd*2)/3;

  e->turn += turnAdd;
}

void _game_dive(GameData* game, int entityIndex, int depth)
{ 
  Entity nullEntity = NULL_ENTITY; 
  FathomData* currentFathom = &game->fathoms[game->current];
  int newFathomIndex = game->current + depth;
  Entity e = currentFathom->entities[entityIndex];
  if(newFathomIndex < 0)
  {
    game_addGlobalMessage("%s are on the surface", e.name);
    return;
  }
  if(newFathomIndex >= MAX_FATHOMS)
  {
    game_addGlobalMessage("%s are on the bottom of the ocean", e.name);
    return;
  }
  currentFathom->entities[entityIndex] = nullEntity;
  int newDepth = game->current+depth;
  if(e.o2depletes)
    e.o2 -= 5;
  if(e.o2 <= 0)
    game_addGlobalMessage("%s drowned while %s", e.name, depth > 0 ? "diving" : "rising");
  else
    game_spawn(&game->fathoms[newDepth], e);

  game->current += depth;
}

void _game_recalcFov(FathomData* fathom)
{
  int i;
  for(i=0; i<MAX_ENTITIES; i++)
  {
    int range = 9;
    if(game_hasCharm(&fathom->entities[i], CHARM_DARKNESS))
    {
      tilemap_forgetSeen(&fathom->tileMap);
      range = 4;
    }
    if(!fathom->entities[i].player)
      continue;
    tilemap_recalcFov(&fathom->tileMap, fathom->entities[i].pos, range);
  }
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

void _game_ai(GameData* game, Entity* e)
{
  FathomData* fathom = &game->fathoms[game->current];
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
  _do_move(fathom, e, move);
}

bool _game_player(GameData* game, Entity* e)
{
  FathomData* fathom = &game->fathoms[game->current];
  if(midFire)
  {
    int i=0;
    for(i=0; i<4; i++)
    {
      if(sys_inputPressed(INPUT_UP+i))
      {
        midFire = false;
        _do_fire(fathom, e, fireIndex, i);
        return true;
      }      
    }
    if(sys_inputPressed(INPUT_ANY))
    {
      game_addGlobalMessage("Nevermind.");
      midFire = false;
    }
  }
  if(midDrop || midUse)
  {
    int i=0;
    for(i=0; i<MAX_INVENTORY; i++)
    {
      if(sys_inputPressed(INPUT_1 + i))
      {
        if(!e->inventory[i].active)
        {
          midDrop = false;
          midUse = false;
          game_addGlobalMessage("No item in slot %d. Nevermind.", i);
          return true;
        }
        bool didAction = false;
        if(midDrop)
          didAction = _do_drop(fathom, e, i);
        if(midUse)
          didAction = _do_use(fathom, e, i);
        midDrop = false;
        midUse = false;
        return didAction;
      }
    }
    if(sys_inputPressed(INPUT_ANY))
    {
      game_addGlobalMessage("Nevermind.");
      midDrop = false;
      midUse = false;      
    }
    return false;
  }

  Point move = NULL_POINT;
  if(sys_inputPressed(INPUT_UP))
    move.y--;
  if(sys_inputPressed(INPUT_RIGHT))
    move.x++;
  if(sys_inputPressed(INPUT_DOWN))
    move.y++;
  if(sys_inputPressed(INPUT_LEFT))
    move.x--;
  if(move.x != 0 || move.y != 0)
  {
    numMessages = 0;
    _do_move(fathom, e, move);
    return true;
  }
  if(sys_inputPressed(INPUT_DIVE))
  {
    numMessages = 0;
    _game_dive(game, 0, 1);
    return true;
  }
  if(sys_inputPressed(INPUT_RISE))
  {
    numMessages = 0;
    _game_dive(game, 0, -1);
    return true;
  }
  if(sys_inputPressed(INPUT_PICKUP))
  {
    numMessages = 0;
    _do_pickup(fathom, e);
    return true;
  }
  if(sys_inputPressed(INPUT_DROP))
  {
    game_addGlobalMessage("Drop which item %d-%d?", 1, MAX_INVENTORY+1);
    midDrop = true;
  }
  if(sys_inputPressed(INPUT_USE))
  {
    game_addGlobalMessage("Use which item %d-%d?", 1, MAX_INVENTORY+1);
    midUse = true;
  }
  return false;
}

bool game_update(GameData* game)
{
  FathomData* fathom = &game->fathoms[game->current];
  Entity* e = &fathom->entities[0];
  if(!e->active)
    return true;
  bool doTurn = !e->player;
  if(e->player)
    doTurn = _game_player(game, e);
  else
    _game_ai(game, e);

  if(doTurn)
  {    
    _do_turn(fathom, e);
    _game_sortEntities(fathom);
    _game_recalcFov(fathom);
  }
  int i;
  bool anyPlayer = false;
  for(i=0; i<MAX_ENTITIES; i++)
  {
    if(fathom->entities[i].player && fathom->entities[i].active)
      anyPlayer = true;
  }
  return fathom->entities[0].player || !anyPlayer;
}

void game_addMessage(const FathomData* fathom, Point p, const char *str, ...)
{
  if(tilemap_visible(&fathom->tileMap, p))
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
}

void game_addGlobalMessage(const char *str, ...)
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
