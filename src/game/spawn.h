typedef enum
{
  ET_SCUBA,
  ET_STARFISH,
  ET_BUBBLE
} EntityType;

Entity spawn_entity(EntityType type);
Item spawn_item(const GameData* game, ItemType type);