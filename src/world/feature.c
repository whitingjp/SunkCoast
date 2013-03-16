#include "main.h"

typedef struct
{
  Point size;
  int minLevel;
  const char* tiles;
} Room;

#define MAX_ROOMS (7)
Room rooms[MAX_ROOMS] = 
{
  { // trapped air
    {6,5},
    0,
    ".####."
    "##oo##"
    "#o  o#"
    "##oo##"
    ".####."
  },
  { // mermaid home
    {13,5},
    8,
    "...   ~   ..."
    ".####~~~####."
    "## ~ #~# ~ ##"
    "## @     @ ##" 
    ".###########."
  },
  { // anemone charmer
    {7,7},
    0,
    ".     ."
    "  #a#  "
    " ##a## "
    " aa=aa "
    " ##a## "
    "  #a#  "
    ".     ."
  },
  {  // kraken den
    {12,8},
    10,
    "#####......."
    "#   ######.."
    "# #  #   ###"
    "# ## # #   #"
    "#  #   ### #"
    "## #####   #"
    "   #.#K  ###"
    "####.#####.."
  },
  { // ink pot
    {6,5},
    3,
    ".###.."
    "# c #."
    "#c c  "
    "# c #."
    ".###.."
  },
  { // canned whiting
    {5,5},
    0,
    "### #"
    "#ww #"
    "#ww# "
    "#ww# "
    "#### "
  },
  { // pillar dance
    {12,10},
    0,
    "....#... ..."
    ".. ...#.#..."
    "..#.#... ..."
    ".. ...#.#.#."
    "..#.#... ..."
    ".. ...#.#.#."
    "#.#.#... ..."
    ".. ...#.#.#."
    "..#.#... ..."
    ".. ...#....."
  }
};




int _feature_enemyTypeFromChar(char c)
{
  int i;
  for(i=0; i<ET_MAX; i++)
  {
    Entity e = spawn_entity(i);
    if(e.character == c)
      return i;
  }
  return -1;
}

void _feature_single(const GameData* game, FathomData* fathom, int level)
{
  TileMap* tileMap = &fathom->tileMap;
  Point offset = NULL_POINT;
  Room room = rooms[sys_randint(MAX_ROOMS)];
  if(room.minLevel > level)
    return;
  offset.x = sys_randint(tileMap->size.x - room.size.x);
  offset.y = sys_randint(tileMap->size.y - room.size.y);
  int i;
  for(i=0; i<room.size.x*room.size.y; i++)
  {
    Point extra = NULL_POINT;
    extra.x = i%room.size.x;
    extra.y = i/room.size.x;
    Point pos = pointAddPoint(offset, extra);
    int tileIndex = tilemap_indexFromTilePosition(tileMap, pos);
    Tile tile = NULL_TILE;
    if(room.tiles[i] == '.')
      continue;
    if(room.tiles[i] == '#')
    {
      tile.frame = getFrameFromAscii('#', 1);
      tile.type = TILE_WALL;
    }
    if(room.tiles[i] == '~')
    {      
      tile.frame = getFrameFromAscii('~', 3);
      tile.type = TILE_HIDE;
    }
    if(room.tiles[i] == '=')
      game_placeAt(fathom, spawn_item(game, IT_CHARM), pos);
    if(room.tiles[i] == '&')
      game_placeAt(fathom, spawn_item(game, IT_CONCH), pos);
    int enemyType = _feature_enemyTypeFromChar(room.tiles[i]);
    tileMap->tiles[tileIndex] = tile;
    if(enemyType != -1)
      game_spawnAt(fathom, spawn_entity(enemyType), pos);
  }
}

void feature_process(const GameData* game, FathomData* fathom, int level)
{
  int num = sys_randint(3)+sys_randint(2);
  int i;
  for(i=0; i<num; i++)
    _feature_single(game, fathom, level);
}