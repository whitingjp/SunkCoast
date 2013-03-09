typedef struct
{
  int x, y;
} Point;
#define NULL_POINT {0,0}

typedef struct
{
  float x, y;
} FPoint;
#define NULL_FPOINT {0.0f,0.0f}

typedef struct
{
  Point a, b;
} Rectangle;
#define NULL_RECTANGLE {NULL_POINT, NULL_POINT}

typedef struct
{
  FPoint a, b;
} FRectangle;
#define NULL_FRECTANGLE {NULL_FPOINT, NULL_FPOINT}

typedef struct
{
  int r,g,b,a;
} Color;
#define NULL_COLOR {0xff,0xff,0xff,0xff}

typedef enum
{
  IMAGE_FONT = 0,
  IMAGE_MAX,
}  ImageID;
#define NULL_IMAGEID (0);

typedef struct
{
  Point pos;
  Point size;
  ImageID image;
} SpriteData;
#define NULL_SPRITEDATA {NULL_POINT, NULL_POINT, 0}

typedef enum
{
  DIR_UP=0,
  DIR_RIGHT,
  DIR_DOWN,
  DIR_LEFT,
} Direction;

typedef enum
{
  TILE_NONE=0,
  TILE_WALL,
  TILE_HIDE,
  TILE_MAX,
} TileType;
#define NULL_TILETYPE (TILE_NONE)

typedef struct
{
  Point frame;
  TileType type;
  bool seen;
  bool visible;
} Tile;
#define NULL_TILE {NULL_POINT, NULL_TILETYPE, false, false}

#define TILEMAP_WIDTH (60)
#define TILEMAP_HEIGHT (20)
extern const Point tilemap_size;
extern const Point tile_size;
typedef struct
{
  SpriteData spriteData;
  Tile tiles[TILEMAP_WIDTH*TILEMAP_HEIGHT];  
  Point size;
  int numTiles;
} TileMap;
#define NULL_TILEMAP (tilemap_null_tileMap());

typedef struct
{
  bool active;
  SpriteData sprite;
  Point frame;
  Point pos;
  int turn;
  bool player;
  int speed;
  int oxygen;
  int strength;
  const char* name;
} Entity;
#define NULL_ENTITY { false, NULL_SPRITEDATA, NULL_POINT, NULL_POINT, INT_MAX, false, 100, 10, 4, NULL}

#define MAX_ENTITIES (64)
typedef struct
{
  Entity entities[MAX_ENTITIES];
  TileMap tileMap;
} GameData;
#define NULL_GAMEDATA (game_null_gamedata());

int min(int a, int b);
int max(int a, int b);
int minmax(int lower, int upper, int n);
Point pointAddPoint(Point a, Point b);
FPoint fpointAddFPoint(FPoint a, FPoint b);
Rectangle rectangleAddPoint(Rectangle a, Point b);
FRectangle frectangleAddFPoint(FRectangle a, FPoint b);
Point pointInverse(Point a);
FPoint fpointInverse(FPoint a);
Point pointMultiply(Point a, int n);
FPoint fpointMultiply(FPoint a, float f);
Rectangle rectangleMultiply(Rectangle a, int n);
FRectangle frectangleMultiply(FRectangle a, float n);

FPoint pointToFPoint(Point in);
Point fpointToPoint(FPoint in);
FRectangle rectangleToFRectangle(Rectangle in);
Rectangle frectangleToRectangle(FRectangle in);

bool rectangleIntersect(Rectangle a, Rectangle b);
bool frectangleIntersect(FRectangle a, FRectangle b);
bool pointInRectangle(Point p, Rectangle r);
bool fpointInFRectangle(FPoint p, FRectangle r);

Point directionToPoint(Direction d);

Point getFrameFromAscii(char c, int colour);
