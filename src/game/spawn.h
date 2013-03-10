typedef enum
{
  ET_STARFISH,
  ET_BUBBLE,
  ET_MAX_ENEMY,
  ET_SCUBA=ET_MAX_ENEMY,
} EntityType;

Entity spawn_entity(EntityType type);
Item spawn_item(const GameData* game, ItemType type);