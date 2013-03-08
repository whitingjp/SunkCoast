#include "main.h"

TileMap tilemap_null_tileMap()
{
  int i;
  TileMap out;
  SpriteData spriteData = {{0,0}, {8,15}, IMAGE_FONT};
  Tile nullTile = NULL_TILE;
  out.spriteData = spriteData;
  out.size.x = TILEMAP_WIDTH;
  out.size.y = TILEMAP_HEIGHT;
  out.numTiles = out.size.x*out.size.y;
  for(i=0; i<out.size.x*out.size.y; i++)
    out.tiles[i] = nullTile;
  return out;
}

void tilemap_draw(TileMap tileMap, Point pos)
{
  int i;
  for(i=0; i<tileMap.numTiles; i++)
  {
    Point tilepos = tilemap_tilePositionFromIndex(&tileMap, i);
    Point renderPos = NULL_POINT;
    renderPos.x = tilepos.x+pos.x;
    renderPos.y = tilepos.y+pos.y;
    sys_drawSprite(tileMap.spriteData, tileMap.tiles[i].frame, renderPos);
  }
}

Point tilemap_tilePositionFromIndex(const TileMap* tileMap, int i)
{
  Point p;
  p.x = i%tileMap->size.x;
  p.y = i/tileMap->size.x;
  return p;
}

int tilemap_indexFromTilePosition(const TileMap* tileMap, Point p)
{
  return p.x + p.y*tileMap->size.x;
}

bool _pointInBounds(const TileMap* tileMap, Point p)
{
  if(p.x < 0)
    return FALSE;
  if(p.y < 0)
    return FALSE;
  if(p.x >= tileMap->size.x)
    return FALSE;
  if(p.y >= tileMap->size.y)
    return FALSE;
  return TRUE;
}

bool tilemap_collides(const TileMap* tileMap, Point p)
{
  int index = tilemap_indexFromTilePosition(tileMap, p);
  if(!_pointInBounds(tileMap, p))
    return TRUE;
  if(tileMap->tiles[index].type == TILE_WALL)
    return TRUE;
  return FALSE;
}

void _tilemap_walker(TileMap* tileMap, int length, Tile tile, TileType notType)
{
  int i;
  Point walker = NULL_POINT;  
  walker.x = sys_randint(tileMap->size.x);
  walker.y = sys_randint(tileMap->size.y);
  int startIndex = tilemap_indexFromTilePosition(tileMap, walker);
  if(tileMap->tiles[startIndex].type == notType)
    return;
  
  for(i=0; i<length; i++)
  {
    int index = tilemap_indexFromTilePosition(tileMap, walker);
    tileMap->tiles[index] = tile;
    Direction dir = sys_randint(4);
    Point move = directionToPoint(dir);
    Point newPoint = pointAddPoint(walker, move);
    if(!_pointInBounds(tileMap, newPoint))
      continue;
    int newIndex = tilemap_indexFromTilePosition(tileMap, newPoint);
    if(tileMap->tiles[newIndex].type == notType)
      continue; 
    walker = newPoint;    
  }
}

TileMap tilemap_generate()
{
  TileMap out = NULL_TILEMAP;
  int i;
  Tile cavern = NULL_TILE;
  cavern.frame = getFrameFromAscii('#', 1);
  cavern.type = TILE_WALL;
  Tile empty = NULL_TILE;
  Tile seaweed = NULL_TILE;
  seaweed.frame = getFrameFromAscii('~', 3);
  for(i=0; i<out.numTiles; i++)
  {
    out.tiles[i] = cavern;
  }
  _tilemap_walker(&out, out.size.x*out.size.y*4, empty, TILE_MAX);
  for(i=0; i<8; i++)
    _tilemap_walker(&out, sys_randint(out.size.x*out.size.y/10), seaweed, TILE_WALL);
  return out;
}