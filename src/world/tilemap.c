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
  if(tileMap->tiles[index].type == TILE_WALL)
    return TRUE;
  return FALSE;
}
