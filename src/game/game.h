typedef struct
{
  SpriteData sprite;
  Point pos;
} Entity;
#define NULL_ENTITY {NULL_SPRITEDATA, NULL_POINT}

#define MAX_ENTITIES (64)
typedef struct
{
  Entity entities[MAX_ENTITIES];
} GameData;
#define NULL_POINT {0,0}