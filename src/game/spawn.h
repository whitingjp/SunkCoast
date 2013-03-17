
typedef enum
{
  ET_ANEMONE,
  ET_TURTLE,    
  ET_WHITEBAIT,
  ET_STARFISH,
  ET_SEAMONKEY,
  ET_CUTTLEFISH,
  ET_PUFFERFISH,
  ET_DOLPHIN,
  ET_MERMAID,
  ET_HYDRA,
  ET_KRAKEN,
  ET_MAX_ENEMY,  
  
  ET_BUBBLE,
  ET_SCUBA,
  ET_MAX,
} EntityType;

Entity spawn_entity(EntityType type);
Item spawn_item(const GameData* game, ItemType type);