UNAME := $(shell uname)
ifeq ($(UNAME), MINGW32_NT-5.1)
	DEVIL_PATH=input/devil
	GLFW_PATH=input/glfw
	GL_LIBS = -lglfw -lglu32 -lopengl32 -lDevIL
	LIB_INCLUDE_PATH = -I$(DEVIL_PATH)/include -I$(GLFW_PATH)/include
	LIB_BIN_PATH = -L$(DEVIL_PATH)/lib -L$(GLFW_PATH)/lib-mingw
else
	GL_LIBS = -lGL -lGLU -lIL -lglfw
	LIB_INCLUDE_PATH = 
	LIB_BIN_PATH =
endif

CC = gcc

CFLAGS = -Isrc $(LIB_INCLUDE_PATH) -Wall -Wextra -Werror -g
LIB_DEPS = $(LIB_BIN_PATH) $(GL_LIBS)

EXE = sunkcoast
CORE_SRC = 
GAME_SRC = 
GAME_HEADERS =
EDITOR_SRC =
EDITOR_HEADERS =

all: $(EXE)

SRC += datatypes.c
SRC += main.c
SRC += game/game.c
SRC += input/libastar/astar.c
SRC += input/libastar/astar_heap.c
SRC += input/libfov/fov.c
SRC += sys/file.c
SRC += sys/logging.c
SRC += sys/sys.c
SRC += world/tilemap.c

HEADERS := $(patsubst %.c,src/%.h,$(SRC)) src/sys/logging.h
OBJ := $(patsubst %.c,obj/%.o,$(SRC))

obj/%.o: src/%.c $(HEADERS)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf obj
	rm -f $(EXE)
	rm -f $(EDITOR)

$(EXE): $(OBJ)
	$(CC) $(OBJ) $(LIB_DEPS) -o $@

