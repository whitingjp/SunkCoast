#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <GLFW/glfw3.h>
#include <IL/il.h> 

#include "datatypes.h"
#include "game/game.h"
#include "game/item.h"
#include "game/spawn.h"
#include "input/libastar/astar.h"
#include "input/libfov/fov.h"
#include "sys/file.h"
#include "sys/logging.h"
#include "sys/sys.h"
#include "world/feature.h"
#include "world/tilemap.h"

Point main_getResolution();
