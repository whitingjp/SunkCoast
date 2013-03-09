TileMap tilemap_null_tileMap();

void tilemap_draw(TileMap tileMap, Point pos);
Point tilemap_tilePositionFromIndex(const TileMap* tileMap, int i);
int tilemap_indexFromTilePosition(const TileMap* tileMap, Point p);
bool tilemap_collides(const TileMap* tileMap, Point p);
TileMap tilemap_generate();

void tilemap_recalcFov(TileMap* tileMap, Point viewer);