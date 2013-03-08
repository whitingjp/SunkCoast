typedef struct
{
  bool active;
  SpriteData sprite;
  Point frame;
  Point pos;
  int turn;
} Entity;
#define NULL_ENTITY { FALSE, NULL_SPRITEDATA, NULL_POINT, NULL_POINT, INT_MAX }

#define MAX_ENTITIES (64)
typedef struct
{
  Entity entities[MAX_ENTITIES];
} GameData;
GameData game_null_gamedata();

void game_spawn(GameData* game, Entity entity);
void game_draw(const GameData* game);
void game_update(GameData* game);

#define NULL_POINT {0,0}