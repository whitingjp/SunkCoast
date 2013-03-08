#include "main.h"

TileMap tilemap_null_tileMap()
{
  int i;
  TileMap out;
  SpriteData nullSpriteData = NULL_SPRITEDATA;
  Tile nullTile = NULL_TILE;
  out.spriteData = nullSpriteData;
  out.size.x = TILEMAP_UNIT_WIDTH;
  out.size.y = TILEMAP_UNIT_HEIGHT;
  out.numTiles = out.size.x*out.size.y;
  for(i=0; i<TILEMAP_MAX_WIDTH*TILEMAP_MAX_HEIGHT; i++)
    out.tiles[i] = nullTile;
  return out;
}

void tilemap_render(TileMap tileMap, Point pos)
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
  if(tileMap->tiles[index].type == TILE_WALL)
    return TRUE;
  return FALSE;
}
