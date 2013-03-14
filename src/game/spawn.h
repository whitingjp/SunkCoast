
typedef enum
{
  ET_WHITEBAIT,
  ET_TURTLE,
  ET_STARFISH,
  ET_SEAMONKEY,
  ET_PUFFERFISH,
  ET_HYDRA,
  ET_KRAKEN,

  ET_MAX_ENEMY,
  ET_BUBBLE,
  ET_SCUBA=ET_MAX_ENEMY,
} EntityType;

Entity spawn_entity(EntityType type);
Item spawn_item(const GameData* game, ItemType type);