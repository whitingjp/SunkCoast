Entity game_null_entity();
FathomData game_null_fathomdata();
GameData game_null_gamedata();

void game_draw(const GameData* game, Point offset);
bool game_update(GameData* game);

void game_place(FathomData* fathom, Item item);
void game_spawn(FathomData* fathom, Entity entity);

void game_addMessage(const FathomData* fathom, Point p, const char *str, ...);
void game_addGlobalMessage(const char *str, ...);

bool game_hasCharm(const Entity *e, CharmSubType charm);