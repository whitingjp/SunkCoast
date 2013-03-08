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

bool tilemap_collides(const TileMap* tileMap, Point p)
{
  int index = tilemap_indexFromTilePosition(tileMap, p);
  if(index < 0 || index >= tileMap->size.x*tileMap->size.y)
    return TRUE;
  if(tileMap->tiles[index].type == TILE_WALL)
    return TRUE;
  return FALSE;
}

TileMap tilemap_generate()
{
  TileMap out = NULL_TILEMAP;
  int i;
  Tile cavern = NULL_TILE;
  cavern.frame = getFrameFromAscii('#', 1);
  cavern.type = TILE_WALL;
  Tile empty = NULL_TILE;
  for(i=0; i<out.numTiles; i++)
  {
    out.tiles[i] = cavern;
  }
  Point walker = NULL_POINT;
  walker.x = out.size.x/4 + sys_randint(out.size.x/2);
  walker.y = out.size.y/4 + sys_randint(out.size.y/2);
  for(i=0; i<(out.size.x*out.size.y)*4; i++)
  {
    int index = tilemap_indexFromTilePosition(&out, walker);
    out.tiles[index] = empty;
    Direction dir = sys_randint(4);
    Point move = directionToPoint(dir);
    Point newPoint = pointAddPoint(walker, move);
    if(newPoint.x >= 0 && newPoint.y >= 0 &&
       newPoint.x < out.size.x && newPoint.y < out.size.y)
    {
      walker = newPoint;
    }
  }
  for(i=0; i<out.numTiles; i++)
  {
    if(out.tiles[i].type == TILE_NONE && sys_randint(4) == 0)
    {
        out.tiles[i].frame = getFrameFromAscii('~', 3);
    }
  }
  return out;
}