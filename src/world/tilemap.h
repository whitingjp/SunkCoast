typedef enum
{
  TILE_NONE=0,
  TILE_WALL,
  TILE_MAX,
} TileType;
#define NULL_TILETYPE (TILE_NONE)

typedef struct
{
  Point frame;
  TileType type;
} Tile;
#define NULL_TILE {NULL_POINT, NULL_TILETYPE}

#define TILEMAP_UNIT_WIDTH (10)
#define TILEMAP_UNIT_HEIGHT (9)
#define TILEMAP_MAX_WIDTH (TILEMAP_UNIT_WIDTH*3)
#define TILEMAP_MAX_HEIGHT (TILEMAP_UNIT_HEIGHT*3)
extern const Point tilemap_size;
extern const Point tile_size;
typedef struct
{
  SpriteData spriteData;
  Tile tiles[TILEMAP_MAX_WIDTH*TILEMAP_MAX_HEIGHT];  
  Point size;
  int numTiles;
} TileMap;
#define NULL_TILEMAP (tilemap_null_tileMap());
TileMap tilemap_null_tileMap();

void tilemap_render(TileMap tileMap, Point pos);
Point tilemap_tilePositionFromIndex(const TileMap* tileMap, int i);
int tilemap_indexFromTilePosition(const TileMap* tileMap, Point p);
bool tilemap_collides(const TileMap* tileMap, Point p);
