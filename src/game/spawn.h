typedef enum
{
  ET_WHITEBAIT,
  ET_STARFISH,

  ET_PUFFERFISH,
  ET_HYDRA,

  ET_MAX_ENEMY,
  ET_BUBBLE,
  ET_SCUBA=ET_MAX_ENEMY,
} EntityType;

Entity spawn_entity(EntityType type);
Item spawn_item(const GameData* game, ItemType type);