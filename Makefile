UNAME := $(shell uname)
ifeq ($(UNAME), MINGW32_NT-5.1)
	DEVIL_PATH=input/devil
	GLFW_PATH=input/glfw
	GL_LIBS = -lglfw -lglu32 -lopengl32 -lDevIL
	LIB_INCLUDE_PATH = -I$(DEVIL_PATH)/include -I$(GLFW_PATH)/include
	LIB_BIN_PATH = -L$(DEVIL_PATH)/lib -L$(GLFW_PATH)/lib-mingw
else
	ifeq ($(UNAME), Darwin)
		GL_LIBS = -framework OpenGL -lIL -lglfw
		LIB_INCLUDE_PATH = 
		LIB_BIN_PATH =
	else
		GL_LIBS = -lGL -lGLU -lIL -lglfw
		LIB_INCLUDE_PATH = 
		LIB_BIN_PATH =
	endif
endif

CC = gcc

CFLAGS = -Isrc $(LIB_INCLUDE_PATH) -Wall -Wextra -Werror -g
LIB_DEPS = $(LIB_BIN_PATH) $(GL_LIBS)

BUILD_DIR = build
OBJ_DIR = build/obj
OUT_DIR=$(BUILD_DIR)/out
DATA_OUT=$(OUT_DIR)/data
DATA=$(DATA_OUT)/font.png

EXE = $(OUT_DIR)/sunkcoast
CORE_SRC = 
GAME_SRC = 
GAME_HEADERS =
EDITOR_SRC =
EDITOR_HEADERS =

all: $(EXE) $(DATA) $(OUT_DIR)/readme.txt

SRC += datatypes.c
SRC += main.c
SRC += game/game.c
SRC += game/item.c
SRC += game/spawn.c
SRC += input/libastar/astar.c
SRC += input/libastar/astar_heap.c
SRC += input/libfov/fov.c
SRC += sys/file.c
SRC += sys/logging.c
SRC += sys/sys.c
SRC += world/feature.c
SRC += world/tilemap.c

HEADERS := $(patsubst %.c,src/%.h,$(SRC)) src/sys/logging.h
OBJ := $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRC))

$(OBJ_DIR)/%.o: src/%.c $(HEADERS)
	mkdir -p $(dir $@)
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf $(BUILD_DIR)

$(EXE): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) $(OBJ) $(LIB_DEPS) -o $@

$(DATA_OUT)/%: data/%
	mkdir -p $(dir $@)
	cp $< $@

$(OUT_DIR)/readme.txt: README
	mkdir -p $(dir $@)
	cp $< $@
