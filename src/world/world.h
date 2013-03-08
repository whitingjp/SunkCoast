typedef struct
{
  TileMap tileMap;
  Rectangle rectangle;
} Screen;
#define NULL_SCREEN ( screen_null() )

#define WORLD_SCREENS_MAX (16)
typedef struct
{
  Screen screens[WORLD_SCREENS_MAX];
  int numScreens;
  Point size;
} World;
#define NULL_WORLD (world_null())

Screen screen_null();
World world_null();
int world_index(const World* world, Point pos);
World world_removeScreen(World world, int screen);
World world_addScreen(World world, Screen screen);
World world_load();
void world_save(const World world);
void world_update();
