#include "main.h"
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

Screen screen_null()
{
  Screen out;
  Rectangle null_rectangle = NULL_RECTANGLE;
  out.tileMap = NULL_TILEMAP;
  out.tileMap.spriteData.image = IMAGE_FONT;
  out.tileMap.spriteData.size.x = 16;
  out.tileMap.spriteData.size.y = 16;
  out.rectangle = null_rectangle;
  return out;
}

World world_null()
{
  int i;
  World out;
  Screen null_screen = NULL_SCREEN;
  out.size.x = 4;
  out.size.y = 4;
  out.numScreens = out.size.x * out.size.y;
  for(i=0; i<out.numScreens; i++)
  {
    Rectangle rect = NULL_RECTANGLE;
    out.screens[i] = null_screen;
    rect.a.x = i%out.size.x;
    rect.a.y = i/out.size.x;
    rect.b.x = rect.a.x+1;
    rect.b.y = rect.a.y+1;
    out.screens[i].rectangle = rect;
  }
  return out;
}

int world_index(const World* world, Point pos)
{
  int i;
  for(i=0; i<world->numScreens; i++)
  {
    if(pointInRectangle(pos, world->screens[i].rectangle))
      return i;
  }
  LOG("Failed to find screen in world at pos %d,%d", pos.x, pos.y);
  return -1;
}

World world_removeScreen(World world, int screen)
{
  int i;
  
  if(screen < 0 || screen > world.numScreens-1)
  {
    LOG("Cannot remove screen %d out of %d", screen, world.numScreens);
    return world;
  }
  
  world.numScreens--;
  for(i=screen; i<world.numScreens; i++)
  {
    world.screens[i] = world.screens[i+1];
  }
  return world;
}

World world_addScreen(World world, Screen screen)
{
  if(world.numScreens >= WORLD_SCREENS_MAX)
  {
    LOG("Cannot add screen to world, already at max: %d", WORLD_SCREENS_MAX);
    return world;
  }
  world.screens[world.numScreens] = screen;
  world.numScreens++;
  return world;
}

World world_load()
{
  World out = NULL_WORLD;
  file_load("data/levels/world", sizeof(World), &out);
  return out;
}

void world_save(const World world)
{
  file_save("data/levels/world", sizeof(World), &world);  
}
