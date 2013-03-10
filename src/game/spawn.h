typedef enum
{
  ET_BUBBLE,
  ET_WHITEBAIT,
  ET_STARFISH,


  ET_PUFFERFISH,

  ET_MAX_ENEMY,
  ET_SCUBA=ET_MAX_ENEMY,
} EntityType;

Entity spawn_entity(EntityType type);
Item spawn_item(const GameData* game, ItemType type);