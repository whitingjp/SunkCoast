#include "main.h"

const Point tile_size = {16, 16};

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
    renderPos.x = tilepos.x*tile_size.x+pos.x;
    renderPos.y = tilepos.y*tile_size.y+pos.y;
    sys_drawSprite(tileMap.spriteData, tileMap.tiles[i].frame, renderPos);
  }
}

void tilemap_renderTypes(TileMap tileMap, Point pos)
{
  int i;
  for(i=0; i<tileMap.numTiles; i++)
  {
    Point tilepos = tilemap_tilePositionFromIndex(&tileMap, i);
    Rectangle rectangle = NULL_RECTANGLE;
    Color col = NULL_COLOR;
    rectangle.a.x = tilepos.x*tile_size.x+pos.x;
    rectangle.a.y = tilepos.y*tile_size.y+pos.y;
    rectangle.b.x = rectangle.a.x+tile_size.x;
    rectangle.b.y = rectangle.a.y+tile_size.y;
    switch(tileMap.tiles[i].type)
    {
      default:
        col.a = 0x00;
        break;
      case TILE_WALL:
        col.r = 0x00; col.b = 0x00; col.g = 0x00; col.a = 0x40;
        break;
    }
    sys_drawRectangle(rectangle, col);
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

bool tilemap_collides(const TileMap* tileMap, Rectangle r)
{
  Rectangle tilesToCheck = NULL_RECTANGLE;
  Point currentTile = NULL_POINT;
  tilesToCheck.a.x =  minmax(0, tileMap->size.x, r.a.x / tile_size.x);
  tilesToCheck.a.y =  minmax(0, tileMap->size.y, r.a.y / tile_size.y);
  tilesToCheck.b.x =  minmax(0, tileMap->size.x, r.b.x / tile_size.x+1);
  tilesToCheck.b.y =  minmax(0, tileMap->size.y, r.b.y / tile_size.y+1);
  for(currentTile.x = tilesToCheck.a.x; currentTile.x < tilesToCheck.b.x; currentTile.x++)
  {
    for(currentTile.y = tilesToCheck.a.y; currentTile.y < tilesToCheck.b.y; currentTile.y++)
    {
      Rectangle tileRect = NULL_RECTANGLE;
      int index = tilemap_indexFromTilePosition(tileMap, currentTile);
      tileRect.a = currentTile;
      tileRect.a.x *= tile_size.x;
      tileRect.a.y *= tile_size.y;
      tileRect.b.x = tileRect.a.x + tile_size.x;
      tileRect.b.y = tileRect.a.y + tile_size.y;
      if(rectangleIntersect(r, tileRect) && tileMap->tiles[index].type == TILE_WALL)
        return TRUE;
    }
  }
  return FALSE;
}
