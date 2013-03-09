#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include <GL/glfw.h>
#include <IL/il.h> 

#include "datatypes.h"
#include "game/game.h"
#include "input/libastar/astar.h"
#include "input/libfov/fov.h"
#include "sys/file.h"
#include "sys/logging.h"
#include "sys/sys.h"
#include "world/tilemap.h"

Point main_getResolution();
