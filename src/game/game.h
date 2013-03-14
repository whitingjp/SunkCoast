Entity game_null_entity();
FathomData game_null_fathomdata();
void game_reset_gamedata(GameData *game);

void game_draw(const GameData* game, Point offset);
bool game_update(GameData* game);

void game_place(FathomData* fathom, Item item);
void game_spawn(FathomData* fathom, Entity entity);
void game_spawnAt(FathomData* fathom, Entity entity, Point p);

void game_addMessage(const FathomData* fathom, Point p, const char *str, ...);
void game_addGlobalMessage(const char *str, ...);

bool game_hasCharm(const Entity *e, CharmSubType charm);
bool game_hurt(FathomData* fathom, Entity *e, int amount);

int game_pointEntityIndex(const FathomData *fathom, Point p);
bool game_pointFree(const FathomData *fathom, Point p);

int game_nextLevel(int level);
