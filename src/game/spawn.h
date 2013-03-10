typedef enum
{
  ST_SCUBA,
  ST_STARFISH,
  ST_BUBBLE
} SpawnType;

Entity spawn_create(SpawnType type);