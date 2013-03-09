GameData game_null_gamedata();
void game_spawn(GameData* game, Entity entity);
void game_draw(const GameData* game, Point offset);
void game_update(GameData* game);

void game_addMessage(const char *str, ...);
